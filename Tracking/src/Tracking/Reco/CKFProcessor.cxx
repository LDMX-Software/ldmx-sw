#include "Tracking/Reco/CKFProcessor.h"

#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "SimCore/Event/SimParticle.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TruthMatchingTool.h"
#include "Tracking/geo/DetectorElement.h"
#include "Tracking/Sim/GeometryContainers.h"

//--- C++ StdLib ---//
#include <algorithm>  //std::vector reverse
#include <iostream>
#include <typeinfo>
// eN files
#include <fstream>

namespace tracking {
namespace reco {

CKFProcessor::CKFProcessor(const std::string& name, framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void CKFProcessor::onNewRun(const ldmx::RunHeader& rh) {
  profiling_map_["setup"] = 0.;
  profiling_map_["hits"] = 0.;
  profiling_map_["seeds"] = 0.;
  profiling_map_["ckf_setup"] = 0.;
  profiling_map_["ckf_run"] = 0.;
  profiling_map_["result_loop"] = 0.;

  // Initialize counters
  nseeds_ = 0;
  ntracks_ = 0;
  eventnr_ = 0;

  // Generate a constant magnetic field
  Acts::Vector3 b_field(0., 0., bfield_ * Acts::UnitConstants::T);

  // Setup a constant magnetic field
  const auto const_b_field = std::make_shared<Acts::ConstantBField>(b_field);

  // Define the target surface - be careful:
  //  x - downstream
  //  y - left (when looking along x)
  //  z - up
  //  Passing identity here means that your target surface is oriented in the
  //  same way
  surf_rotation_ = Acts::RotationMatrix3::Zero();
  // u direction along +Y
  surf_rotation_(1, 0) = 1;
  // v direction along +Z
  surf_rotation_(2, 1) = 1;
  // w direction along +X
  surf_rotation_(0, 2) = 1;

  Acts::Vector3 target_pos(0., 0., 0.);
  Acts::Translation3 target_translation(target_pos);
  Acts::Transform3 target_transform(target_translation * surf_rotation_);

  // Unbounded surface
  target_surface_ =
      Acts::Surface::makeShared<Acts::PlaneSurface>(target_transform);

  // Setup a interpolated bfield map
  if (field_map_.empty())
    loadBField(map_offset_);
  else
    loadBField(field_map_, map_offset_);
  const auto map =
      std::static_pointer_cast<InterpolatedMagneticField3>(bField());

  auto acts_logging_level = Acts::Logging::FATAL;
  if (debug_acts_) acts_logging_level = Acts::Logging::VERBOSE;

  // Setup the steppers
  const auto stepper = Acts::EigenStepper<>{map};
  const auto const_stepper = Acts::EigenStepper<>{const_b_field};
  const auto multi_stepper = Acts::MultiEigenStepperLoop{map};

  // Setup the navigator
  Acts::Navigator::Config nav_cfg{geometry().getTG()};
  nav_cfg.resolveMaterial = true;
  nav_cfg.resolvePassive = true;
  nav_cfg.resolveSensitive = true;
  const Acts::Navigator navigator(nav_cfg);

  propagator_ = std::make_unique<CkfPropagator>(
      stepper, navigator,
      Acts::getDefaultLogger("CKF_PROP", acts_logging_level));

  // Setup the finder / fitters
  ckf_ = std::make_unique<std::decay_t<decltype(*ckf_)>>(
      *propagator_, Acts::getDefaultLogger("CKF", acts_logging_level));
  // Extrapolation uses VoidNavigator so it can reach surfaces outside the
  // tracking geometry (e.g. ECAL scoring plane) without being stopped at
  // volume boundaries.
  propagator_extrap_ = std::make_unique<ExtrapPropagator>(
      Acts::EigenStepper<>{map}, Acts::VoidNavigator{});
  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(
      *propagator_extrap_, geometryContext(), magneticFieldContext());

  // Setup zero-B CKF as fallback
  Acts::ConstantBField zero_b_field(Acts::Vector3(0., 0., 0.));
  const auto zero_b_stepper = Acts::EigenStepper<>{
      std::make_shared<Acts::ConstantBField>(zero_b_field)};
  propagator_zero_b_ =
      std::make_unique<CkfPropagator>(zero_b_stepper, navigator);
  ckf_zero_b_ = std::make_unique<std::decay_t<decltype(*ckf_zero_b_)>>(
      *propagator_zero_b_,
      Acts::getDefaultLogger("CKF_ZERO_B", acts_logging_level));
  propagator_extrap_zero_b_ = std::make_unique<ExtrapPropagator>(
      Acts::EigenStepper<>{std::make_shared<Acts::ConstantBField>(zero_b_field)},
      Acts::VoidNavigator{});
  trk_extrap_zero_b_ =
      std::make_shared<std::decay_t<decltype(*trk_extrap_zero_b_)>>(
          *propagator_extrap_zero_b_, geometryContext(), magneticFieldContext());

  // Setup const-B (1.5T) CKF as fallback for tagger
  propagator_const_b_ =
      std::make_unique<CkfPropagator>(const_stepper, navigator);
  ckf_const_b_ = std::make_unique<std::decay_t<decltype(*ckf_const_b_)>>(
      *propagator_const_b_,
      Acts::getDefaultLogger("CKF_CONST_B", acts_logging_level));
  propagator_extrap_const_b_ = std::make_unique<ExtrapPropagator>(
      Acts::EigenStepper<>{const_b_field}, Acts::VoidNavigator{});
  trk_extrap_const_b_ =
      std::make_shared<std::decay_t<decltype(*trk_extrap_const_b_)>>(
          *propagator_extrap_const_b_, geometryContext(), magneticFieldContext());
}  // end of CKFProcessor::onNewRun()

void CKFProcessor::produce(framework::Event& event) {
  eventnr_++;
  // get the tracking geometry from conditions
  auto tg{geometry()};

  // TODO use global variable instead and call clear;

  std::vector<ldmx::Track> tracks;

  auto start = std::chrono::high_resolution_clock::now();

  nevents_++;

  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("LDMX Tracking Geometry Maker",
                                           Acts::Logging::DEBUG));

  // Move this at the start of the producer
  Acts::PropagatorOptions<Acts::StepperPlainOptions,
                          Acts::NavigatorPlainOptions, ActionList>
      propagator_options(geometryContext(), magneticFieldContext());

  propagator_options.pathLimit = std::numeric_limits<double>::max();
  // Activate loop protection at some pt value
  propagator_options.loopProtection = false;
  //(startParameters.transverseMomentum() < cfg.ptLoopers);

  // Switch the material interaction on/off & eventually into logging mode
  auto& m_interactor =
      propagator_options.actorList.get<Acts::MaterialInteractor>();
  m_interactor.multipleScattering = true;
  m_interactor.energyLoss = true;
  m_interactor.recordInteractions = false;

  // The logger can be switched to sterile, e.g. for timing logging
  auto& s_logger =
      propagator_options.actorList.get<Acts::detail::SteppingLogger>();
  s_logger.sterile = true;
  // Set a maximum step size
  propagator_options.stepping.maxStepSize =
      propagator_step_size_ * Acts::UnitConstants::mm;
  propagator_options.maxSteps = propagator_max_steps_;

  // #######################//
  // Kalman Filter algorithm//
  // #######################//

  // Step 1 - Form the source links

  // a) Loop over the sim Hits

  auto setup = std::chrono::high_resolution_clock::now();
  profiling_map_["setup"] +=
      std::chrono::duration<double, std::milli>(setup - start).count();

  const auto& measurements = event.getCollection<ldmx::Measurement>(
      measurement_collection_, input_pass_name_);

  // check if SimParticleMap is available for truth matching
  std::shared_ptr<tracking::sim::TruthMatchingTool> truth_matching_tool =
      nullptr;
  std::map<int, ldmx::SimParticle> particle_map;

  if (event.exists(sim_particles_coll_name_, sim_particles_event_passname_)) {
    ldmx_log(debug) << "Setting up track truth matching tool";
    particle_map = event.getMap<int, ldmx::SimParticle>(
        sim_particles_coll_name_, sim_particles_event_passname_);
    truth_matching_tool = std::make_shared<tracking::sim::TruthMatchingTool>(
        particle_map, measurements);
  }

  // The mapping between the geometry identifier
  // and the IndexsourceLink that points to the hit
  const auto geo_id_sl_map = makeGeoIdSourceLinkMap(tg, measurements);

  auto hits = std::chrono::high_resolution_clock::now();
  profiling_map_["hits"] +=
      std::chrono::duration<double, std::milli>(hits - setup).count();

  // ============   Setup the CKF  ============

  // Retrieve the seeds
  const auto& seed_tracks =
      event.getCollection<ldmx::Track>(seed_coll_name_, input_pass_name_);

  ldmx_log(info) << "Number of " << seed_coll_name_
                 << " seed tracks = " << seed_tracks.size();

  if (seed_tracks.empty()) {
    std::vector<ldmx::Track> empty;
    ldmx_log(warn) << "No seed tracks, returning...";
    event.add(out_trk_collection_, empty);
    return;
  }

  // Run the CKF on each seed and produce a track candidate
  std::vector<Acts::BoundTrackParameters> start_parameters;

  ldmx_log(debug) << "Transform the seed track to bound parameters";
  int seed_track_index{0};
  for (auto& seed : seed_tracks) {
    // Transform the seed track to bound parameters.
    // Perigee is stored in LDMX global frame; convert to ACTS frame for
    // surface.
    Acts::Vector3 perigee_acts = tracking::sim::utils::ldmx2Acts(Acts::Vector3(
        seed.getPerigeeX(), seed.getPerigeeY(), seed.getPerigeeZ()));
    std::shared_ptr<Acts::PerigeeSurface> perigee_surface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(perigee_acts);

    Acts::BoundVector param_vec;
    param_vec << seed.getD0(), seed.getZ0(), seed.getPhi(), seed.getTheta(),
        seed.getQoP(), seed.getT();

    Acts::BoundMatrix cov_mat =
        tracking::sim::utils::unpackCov(seed.getPerigeeCov());

    ldmx_log(debug) << "  For seed index_ = " << seed_track_index
                    << ": Perigee X / Y / Z = " << seed.getPerigeeX() << " / "
                    << seed.getPerigeeY() << " / " << seed.getPerigeeZ()
                    << ", D0 = " << param_vec[0] << ", Z0 = " << param_vec[1]
                    << ", Phi = " << param_vec[2]
                    << ", Theta = " << param_vec[3]
                    << ", QoP = " << param_vec[4]
                    << ", Time = " << param_vec[5];

    ldmx_log(debug) << "  Cov matrix diagonal (" << cov_mat(0, 0) << ", "
                    << cov_mat(1, 1) << ", " << cov_mat(2, 2) << ")";

    // need to set particle hypothesis...set to electron for now...
    auto part_hypo{Acts::ParticleHypothesis::electron()};
    start_parameters.push_back(Acts::BoundTrackParameters(
        perigee_surface, param_vec, cov_mat, part_hypo));

    // This is a global variable for performance checks
    nseeds_++;
    // This is just to index_ the seed we are looking at
    seed_track_index++;
  }  // loop on seeds

  auto seeds = std::chrono::high_resolution_clock::now();
  profiling_map_["seeds"] +=
      std::chrono::duration<double, std::milli>(seeds - hits).count();

  Acts::GainMatrixUpdater kf_updater;

  // configuration for the measurement selector. Empty geometry identifier means
  // applicable to all the detector elements

  Acts::MeasurementSelector::Config measurement_selector_cfg = {
      // global default: no chi2 cut, only one measurement per surface
      {Acts::GeometryIdentifier(), {{}, {outlier_pval_}, {1u}}},
  };

  Acts::MeasurementSelector meas_sel{measurement_selector_cfg};

  tracking::sim::LdmxMeasurementCalibrator calibrator{measurements};

  // Create source link accessor iterator type and lambda
  struct SourceLinkAccIt {
    using BaseIt = decltype(geo_id_sl_map.begin());
    BaseIt it_;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

    using difference_type = typename BaseIt::difference_type;
    using iterator_category = typename BaseIt::iterator_category;
    using value_type = Acts::SourceLink;
    using pointer = typename BaseIt::pointer;
    using reference = value_type&;
#pragma GCC diagnostic pop

    SourceLinkAccIt& operator++() {
      ++it_;
      return *this;
    }
    bool operator==(const SourceLinkAccIt& other) const {
      return it_ == other.it_;
    }
    bool operator!=(const SourceLinkAccIt& other) const {
      return !(*this == other);
    }
    value_type operator*() const { return value_type{it_->second}; }
  };

  auto source_link_accessor = [&](const Acts::Surface& surface)
      -> std::pair<SourceLinkAccIt, SourceLinkAccIt> {
    auto [begin, end] = geo_id_sl_map.equal_range(surface.geometryId());
    return {SourceLinkAccIt{begin}, SourceLinkAccIt{end}};
  };

  // v46: calibrator and measurementSelector moved to TrackStateCreator
  Acts::TrackStateCreator<SourceLinkAccIt, TrackContainer> track_state_creator;
  track_state_creator.sourceLinkAccessor
      .connect<&decltype(source_link_accessor)::operator(),
               decltype(source_link_accessor)>(&source_link_accessor);
  if (use1_dmeasurements_) {
    track_state_creator.calibrator
        .connect<&tracking::sim::LdmxMeasurementCalibrator::calibrate1d<
            Acts::VectorMultiTrajectory>>(&calibrator);
  } else {
    track_state_creator.calibrator
        .connect<&tracking::sim::LdmxMeasurementCalibrator::calibrate<
            Acts::VectorMultiTrajectory>>(&calibrator);
  }
  track_state_creator.measurementSelector
      .connect<&Acts::MeasurementSelector::select<Acts::VectorMultiTrajectory>>(
          &meas_sel);

  Acts::CombinatorialKalmanFilterExtensions<TrackContainer> ckf_extensions;
  ckf_extensions.updater.connect<
      &Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(
      &kf_updater);
  ckf_extensions.createTrackStates
      .connect<&Acts::TrackStateCreator<SourceLinkAccIt,
                                        TrackContainer>::createTrackStates>(
          &track_state_creator);

  ldmx_log(debug) << "Setting up surfaces...";

  std::shared_ptr<const Acts::PerigeeSurface> origin_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(0., 0., 0.));

  ldmx_log(debug) << "About to run CKF...";

  // run the CKF for all initial track states
  auto ckf_setup = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_setup"] +=
      std::chrono::duration<double, std::milli>(ckf_setup - seeds).count();

  Acts::VectorTrackContainer vtc;
  Acts::VectorMultiTrajectory mtj;
  Acts::TrackContainer tc{vtc, mtj};

  // The number of track candidates (i.e. startParameters.size()) is always
  // the same as the number of seed tracks
  ldmx_log(debug) << "Loop on the track candidates";
  for (size_t track_id = 0u; track_id < start_parameters.size(); ++track_id) {
    ldmx_log(debug) << "---------------------------";
    ldmx_log(debug) << "Candidate Track ID = " << track_id;
    // Define the CKF options here:
    const Acts::CombinatorialKalmanFilterOptions<TrackContainer>
        ckf_options(TrackingGeometryUser::geometryContext(),
                    TrackingGeometryUser::magneticFieldContext(),
                    TrackingGeometryUser::calibrationContext(),
                    ckf_extensions,
                    static_cast<Acts::PropagatorPlainOptions>(propagator_options),
                    true /* multiple scattering */,
                    false /* energy loss */);

    ldmx_log(debug) << "  Checking options:  multiple scattering = "
                    << ckf_options.multipleScattering
                    << "  energy loss = " << ckf_options.energyLoss;

    // Try field-map CKF first
    auto results =
        ckf_->findTracks(start_parameters.at(track_id), ckf_options, tc);

    auto start_params = start_parameters.at(track_id).parameters().transpose();

    // If field-map CKF fails, try appropriate fallback based on tracking system
    if (!results.ok()) {
      if (!tagger_tracking_) {
        // Recoil tracking: try zero-B CKF as fallback
        n_fieldmap_ckf_failed_recoil_++;
        ldmx_log(debug)
            << "    Field-map CKF failed, trying zero-B CKF fallback";
        results = ckf_zero_b_->findTracks(start_parameters.at(track_id),
                                          ckf_options, tc);
        if (results.ok()) {
          n_zerob_ckf_recovered_recoil_++;
          ldmx_log(debug) << "    Yay! Zero-B CKF succeeded as fallback!";
        } else {
          ldmx_log(debug) << "    Zero-B CKF also failed!";
        }
      } else {
        // Tagger tracking: try const-B (1.5T) CKF as fallback
        n_fieldmap_ckf_failed_tagger_++;
        ldmx_log(debug)
            << "    Field-map CKF failed, trying const-B (1.5T) CKF fallback";
        results = ckf_const_b_->findTracks(start_parameters.at(track_id),
                                           ckf_options, tc);
        if (results.ok()) {
          n_constb_ckf_recovered_tagger_++;
          ldmx_log(debug) << "    Yay! Const-B CKF succeeded as fallback!";
        } else {
          ldmx_log(debug) << "    Const-B CKF also failed!";
        }
      }
    }

    ldmx_log(debug)
        << "  Checking CKF success for track candidate with params: "
        << " D0 = " << start_params[0] << " Z0 = " << start_params[1]
        << ", Phi = " << start_params[2] << " Theta = " << start_params[3]
        << ", QoP = " << start_params[4] << " Time = " << start_params[5];
    if (not results.ok()) {
      ldmx_log(debug) << "    CKF failed!";
      continue;
    } else {
      ldmx_log(debug) << "    CKF succeded!";
    }

    auto& tracks_from_seed = results.value();
    if (tracks_from_seed.size() != 1) {
      ldmx_log(info) << "  tracksFromSeed.size = " << tracks_from_seed.size();
    }
    // For now it seems this loop is only looping on a single element
    for (auto& track : tracks_from_seed) {
      // do the track smoothing...this is not done in the CKF code anymore
      auto smooth_result = Acts::smoothTrack(geometryContext(), track);
      if (!smooth_result.ok()) {
        ldmx_log(warn) << "smoothTrack failed: "
                       << smooth_result.error().message();
      }
      // Build the output Track
      ldmx::Track trk;

      // Extrapolate to the target surface
      auto opt_target = trk_extrap_->extrapolate(track, target_surface_);

      if (!opt_target) {
        if (tagger_tracking_) {
          n_fieldmap_target_extrap_failed_tagger_++;
          ldmx_log(debug) << "    Field-map target extrapolation failed, "
                             "trying const-B (1.5T) fallback";
          opt_target = trk_extrap_const_b_->extrapolate(track, target_surface_);
          if (opt_target)
            n_constb_target_extrap_recovered_tagger_++;
          else
            ldmx_log(debug) << "    Both field-map and Const-B target "
                               "extrapolation failed!";
        } else {
          n_fieldmap_target_extrap_failed_recoil_++;
          ldmx_log(debug) << "    Field-map target extrapolation failed, "
                             "trying zero-B fallback";
          opt_target = trk_extrap_zero_b_->extrapolate(track, target_surface_);
          if (opt_target)
            n_zerob_target_extrap_recovered_recoil_++;
          else
            ldmx_log(debug)
                << "    Both field-map and Zero-B target extrapolation failed!";
        }
      }

      if (!opt_target) {
        ldmx_log(debug) << "    Could not extrapolate to target! nhits = "
                        << track.nMeasurements() << " Printing track states:";
        for (const auto ts : track.trackStatesReversed()) {
          if (ts.hasSmoothed())
            ldmx_log(debug) << "    Parameters: " << ts.smoothed().transpose();
          else
            ldmx_log(debug) << "    Track state not smoothed!";
        }
        ldmx_log(debug) << "    ...skipping this track candidate...";
        continue;
      }

      ldmx_log(debug) << "    Successfully obtained TrackState at target";

      // Build TrackState in LDMX coordinates and add to track
      auto ts_at_target = tracking::sim::utils::makeTrackState(
          geometryContext(), *opt_target, ldmx::AtTarget);
      trk.addTrackState(ts_at_target);

      ldmx_log(debug) << "    Position at target (LDMX): ("
                      << ts_at_target.pos_[0] << ", " << ts_at_target.pos_[1]
                      << ", " << ts_at_target.pos_[2] << ") mm"
                      << "  Momentum: (" << ts_at_target.mom_[0] << ", "
                      << ts_at_target.mom_[1] << ", " << ts_at_target.mom_[2]
                      << ") GeV";

      // Update ACTS track reference surface (needed for downstream ACTS usage)
      track.setReferenceSurface(target_surface_);
      track.parameters() = opt_target->parameters();

      // Store perigee (bound) parameters at the target for convenience
      trk.setPerigeeParameters(tracking::sim::utils::convertActsToLdmxPars(
          opt_target->parameters()));
      if (opt_target->covariance()) {
        std::vector<double> cov_vec;
        tracking::sim::utils::flatCov(*(opt_target->covariance()), cov_vec);
        trk.setPerigeeCov(cov_vec);
      }
      // Perigee location: target surface origin rotated to LDMX frame
      Acts::Vector3 target_loc_ldmx = tracking::sim::utils::acts2Ldmx(
          target_surface_->localToGlobalTransform(geometryContext()).translation());
      trk.setPerigeeLocation(target_loc_ldmx[0], target_loc_ldmx[1],
                             target_loc_ldmx[2]);

      trk.setChi2(track.chi2());
      trk.setNhits(track.nMeasurements());
      trk.setNdf(track.nMeasurements() - 5);
      trk.setNsharedHits(track.nSharedHits());
      trk.setCharge(opt_target->parameters()[Acts::eBoundQOverP] > 0 ? 1 : -1);

      // At least min_hits hits and p > 50 MeV
      if ((trk.getNhits() <= min_hits_) ||
          (std::abs(1. / trk.getQoP()) <= 0.05)) {
        ldmx_log(debug)
            << "  > Track candidate did NOT meet the requirements: Nhits = "
            << trk.getNhits() << " and p = " << std::abs(1. / trk.getQoP())
            << " GeV";
        continue;
      }

      // Add measurements to the final track
      ldmx_log(debug) << "  Add measurements to the final track from "
                      << track.nTrackStates() << " TrackStates with "
                      << track.nMeasurements() << " measurements";

      int trk_state_index{0};
      for (const auto ts : track.trackStatesReversed()) {
        // Check TrackStates Quality
        ldmx_log(debug) << "    Checking Track State index_ = "
                        << trk_state_index << " at location "
                        << ts.referenceSurface()
                               .localToGlobalTransform(geometryContext())
                               .translation()
                               .transpose();

        if (ts.hasSmoothed()) {
          ldmx_log(debug) << "    Smoothed track parameters: "
                          << ts.smoothed().transpose();
          // ldmx_log(debug) << "    Smoothed covariance mtx:\n" <<
          // ts.smoothedCovariance();
        }

        // Check if the track state is a measurement
        auto type_flags = ts.typeFlags();

        if (type_flags.isMeasurement() &&
            ts.hasUncalibratedSourceLink()) {
          Acts::SourceLink usl = ts.getUncalibratedSourceLink();
          const acts_examples::IndexSourceLink& sl =
              usl.get<acts_examples::IndexSourceLink>();

          ldmx::Measurement ldmx_meas = measurements.at(sl.index());
          ldmx_log(debug) << "    Adding measurement to ldmx::track with "
                             "source link index_ = "
                          << sl.index();
          ldmx_log(trace) << "    Measurement:\n" << ldmx_meas;
          trk.addMeasurementIndex(sl.index());

          // Store the smoothed state for algebraic unbiased residuals in the
          // DQM.  The leave-one-out formula (NIM A 262, 444, 1987) removes
          // this hit's contribution analytically:
          //   r_ubs = V/(V - C) * (m - x_smooth),  pull = r_ubs * sqrt(V-C)/V
          // This works correctly for all layers, including seed layers where
          // the predicted state would be biased.
          if (ts.hasSmoothed()) {
            trk.addSmoothedLoc0(
                static_cast<float>(ts.smoothed()[Acts::eBoundLoc0]),
                static_cast<float>(ts.smoothedCovariance()(Acts::eBoundLoc0,
                                                           Acts::eBoundLoc0)));
          }

          // Extract path length from the track state based on the angle
          if (ts.hasSmoothed()) {
            const auto& meas_surface = ts.referenceSurface();
            const auto& smoothed_params = ts.smoothed();

            // Get the momentum from the track parameters
            // momentum = p * direction where direction = (sin(theta)*cos(phi),
            // sin(theta)*sin(phi), cos(theta))
            float p_inv = smoothed_params[Acts::eBoundQOverP];
            float p = 1.0f / std::abs(p_inv);
            float theta = smoothed_params[Acts::eBoundTheta];
            float phi = smoothed_params[Acts::eBoundPhi];

            Acts::Vector3 global_momentum(p * std::sin(theta) * std::cos(phi),
                                          p * std::sin(theta) * std::sin(phi),
                                          p * std::cos(theta));

            // Get the local frame (transform from global to local)
            auto local_frame_transform =
                meas_surface.localToGlobalTransform(geometryContext());
            Acts::Vector3 local_momentum =
                local_frame_transform.rotation().transpose() * global_momentum;

            // Calculate local angle components (tangent of angles)
            float phi_u = (local_momentum.z() != 0)
                              ? local_momentum.x() / local_momentum.z()
                              : 0.;
            float phi_v = (local_momentum.z() != 0)
                              ? local_momentum.y() / local_momentum.z()
                              : 0.;

            // Calculate the total angle from the local angle components
            // tan(angle) = sqrt(phi_u^2 + phi_v^2)
            // cos(angle) = 1 / sqrt(1 + tan(angle)^2)
            // path_length = thickness / cos(angle)
            float sensor_thickness = 0.0f;
            if (const auto* placement = meas_surface.surfacePlacement()) {
              sensor_thickness = static_cast<float>(
                  static_cast<const tracking::geo::DetectorElement*>(placement)
                      ->thickness());
            } else {
              ldmx_log(warn) << "No detector element for measurement surface"
                             << " — skipping dE/dx for this hit";
              continue;
            }
            float tan_angle_sq = phi_u * phi_u + phi_v * phi_v;
            float cos_angle = 1.0f / std::sqrt(1.0f + tan_angle_sq);
            float path_length = sensor_thickness / cos_angle;

            ldmx_log(debug) << "      Local angles: phi_u = " << phi_u
                            << ", phi_v = " << phi_v
                            << "; Path length = " << path_length << " mm";

            // Calculate dE/dx and add to track (in MeV/mm)
            float edep = ldmx_meas.getEdep();
            float dedx = edep / path_length;
            trk.addDedxMeasurement(dedx);

            ldmx_log(debug) << "      Edep = " << edep
                            << " MeV, dE/dx = " << dedx << " MeV/mm";
          }
        } else {
          ldmx_log(debug) << "    This TrackState is not a measurement";
        }
        trk_state_index++;
      }

      ldmx_log(debug) << "  Starting extrapolations";
      // Extrapolations
      // To ECAL
      const double ecal_scoring_plane = 240.5;
      Acts::Vector3 pos(ecal_scoring_plane, 0., 0.);
      Acts::Translation3 surf_translation(pos);
      Acts::Transform3 surf_transform(surf_translation * surf_rotation_);
      const std::shared_ptr<Acts::PlaneSurface> ecal_surface =
          Acts::Surface::makeShared<Acts::PlaneSurface>(surf_transform);

      // Beam Origin unbounded surface
      const std::shared_ptr<Acts::Surface> beam_origin_surface =
          tracking::sim::utils::unboundSurface(-700);

      if (tagger_tracking_) {
        ldmx_log(debug) << "    Beam Origin Extrapolation";
        auto opt_beam_origin =
            trk_extrap_->extrapolate(track, beam_origin_surface);
        if (opt_beam_origin) {
          trk.addTrackState(tracking::sim::utils::makeTrackState(
              geometryContext(), *opt_beam_origin, ldmx::AtBeamOrigin));
          ldmx_log(debug)
              << "    Successfully obtained TrackState at beam origin";
        }
      }

      // Recoil Extrapolation to ECAL only
      if (!tagger_tracking_) {
        ldmx_log(debug) << "    Ecal Extrapolation";
        auto opt_ecal = trk_extrap_->extrapolate(track, ecal_surface);

        if (!opt_ecal) {
          n_fieldmap_ecal_extrap_failed_recoil_++;
          ldmx_log(debug) << "    Field-map ECAL extrapolation failed, trying "
                             "zero-B fallback";
          opt_ecal = trk_extrap_zero_b_->extrapolate(track, ecal_surface);
          if (opt_ecal)
            n_zerob_ecal_extrap_recovered_recoil_++;
          else
            ldmx_log(debug)
                << "    Both field-map and Zero-B ECAL extrapolation failed!";
        }

        if (opt_ecal) {
          auto ts_at_ecal = tracking::sim::utils::makeTrackState(
              geometryContext(), *opt_ecal, ldmx::AtECAL);
          trk.addTrackState(ts_at_ecal);
          ldmx_log(debug) << "    Successfully obtained TrackState at ECAL";
          ldmx_log(debug) << "    Position at ECAL (LDMX): ("
                          << ts_at_ecal.pos_[0] << ", " << ts_at_ecal.pos_[1]
                          << ", " << ts_at_ecal.pos_[2] << ") mm";
        }
      }

      // Truth matching
      if (truth_matching_tool) {
        auto truth_info = truth_matching_tool->truthMatch(trk);
        trk.setTrackID(truth_info.track_id_);
        trk.setPdgID(truth_info.pdg_id_);
        trk.setTruthProb(truth_info.truth_prob_);
      }

      // Adding the track candidate to the track collection
      ldmx_log(debug)
          << "  > Adding the track candidate to the track collection";
      tracks.push_back(trk);
      ntracks_++;
    }  // // loop on tracksFromSeed (which usually has 1 element)
  }  // loop seed track parameters (i.e. track candidates)

  ldmx_log(info) << "Number of CKF tracks " << tracks.size();

  auto ckf_run = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_run"] +=
      std::chrono::duration<double, std::milli>(ckf_run - ckf_setup).count();

  // Calculating Shared Hits
  auto shared_hits = computeSharedHits(
      tracks, measurements, tg, tracking::sim::utils::sourceLinkHash,
      tracking::sim::utils::sourceLinkEquality);
  for (std::size_t i_track = 0; i_track < shared_hits.size(); ++i_track) {
    tracks[i_track].setNsharedHits(shared_hits[i_track].size());
    for (auto idx : shared_hits[i_track]) {
      tracks[i_track].addSharedIndex(idx);
    }
  }

  auto result_loop = std::chrono::high_resolution_clock::now();
  profiling_map_["result_loop"] +=
      std::chrono::duration<double, std::milli>(result_loop - ckf_run).count();

  // Add the tracks to the event
  event.add(out_trk_collection_, tracks);

  auto end = std::chrono::high_resolution_clock::now();
  // long long microseconds =
  // std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
}  // end of produce()

void CKFProcessor::onProcessStart() {
  if (use1_dmeasurements_)
    ldmx_log(debug) << "Use1Dmeasurements = " << std::boolalpha
                    << use1_dmeasurements_;
  if (remove_stereo_)
    ldmx_log(debug) << "Remove_stereo = " << std::boolalpha << remove_stereo_;
}

void CKFProcessor::onProcessEnd() {
  ldmx_log(info) << "--------------------------------- ";
  ldmx_log(info) << "Found " << ntracks_ << " tracks  / " << nseeds_
                 << " nseeds";
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / nevents_ << " ms";
  ldmx_log(info) << "Breakdown::";
  ldmx_log(info) << "  setup       Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["setup"] / nevents_
                 << " ms";
  ldmx_log(info) << "  hits        Avg Time/Event = " << std::fixed
                 << std::setprecision(2) << profiling_map_["hits"] / nevents_
                 << " ms";
  ldmx_log(info) << "  seeds       Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["seeds"] / nevents_
                 << " ms";
  ldmx_log(info) << "  ckf_setup    Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["ckf_setup"] / nevents_ << " ms";
  ldmx_log(info) << "  ckf_run     Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["ckf_run"] / nevents_
                 << " ms";
  ldmx_log(info) << "  result_loop Avg Time/Event = " << std::fixed
                 << std::setprecision(1)
                 << profiling_map_["result_loop"] / nevents_ << " ms";

  // CKF fallback statistics
  ldmx_log(info) << "CKF Fallback Statistics::";
  if (tagger_tracking_) {
    ldmx_log(info) << "  Tagger: Field-map CKF failed "
                   << n_fieldmap_ckf_failed_tagger_
                   << " times, const-B CKF recovered "
                   << n_constb_ckf_recovered_tagger_ << " ("
                   << (n_fieldmap_ckf_failed_tagger_ > 0
                           ? 100.0 * n_constb_ckf_recovered_tagger_ /
                                 n_fieldmap_ckf_failed_tagger_
                           : 0.0)
                   << "%)";

    // Extrapolation fallback statistics for tagger
    ldmx_log(info) << "Extrapolation Fallback Statistics::";
    ldmx_log(info) << "  Tagger Target: Field-map extrap failed "
                   << n_fieldmap_target_extrap_failed_tagger_
                   << " times, const-B extrap recovered "
                   << n_constb_target_extrap_recovered_tagger_ << " ("
                   << (n_fieldmap_target_extrap_failed_tagger_ > 0
                           ? 100.0 * n_constb_target_extrap_recovered_tagger_ /
                                 n_fieldmap_target_extrap_failed_tagger_
                           : 0.0)
                   << "%)";
  }

  if (!tagger_tracking_) {
    ldmx_log(info) << "  Recoil: Field-map CKF failed "
                   << n_fieldmap_ckf_failed_recoil_
                   << " times, zero-B CKF recovered "
                   << n_zerob_ckf_recovered_recoil_ << " ("
                   << (n_fieldmap_ckf_failed_recoil_ > 0
                           ? 100.0 * n_zerob_ckf_recovered_recoil_ /
                                 n_fieldmap_ckf_failed_recoil_
                           : 0.0)
                   << "%)";

    // Extrapolation fallback statistics
    ldmx_log(info) << "Extrapolation Fallback Statistics::";
    ldmx_log(info) << "  Recoil Target: Field-map extrap failed "
                   << n_fieldmap_target_extrap_failed_recoil_
                   << " times, zero-B extrap recovered "
                   << n_zerob_target_extrap_recovered_recoil_ << " ("
                   << (n_fieldmap_target_extrap_failed_recoil_ > 0
                           ? 100.0 * n_zerob_target_extrap_recovered_recoil_ /
                                 n_fieldmap_target_extrap_failed_recoil_
                           : 0.0)
                   << "%)";
    ldmx_log(info) << "  Recoil ECAL: Field-map extrap failed "
                   << n_fieldmap_ecal_extrap_failed_recoil_
                   << " times, zero-B extrap recovered "
                   << n_zerob_ecal_extrap_recovered_recoil_ << " ("
                   << (n_fieldmap_ecal_extrap_failed_recoil_ > 0
                           ? 100.0 * n_zerob_ecal_extrap_recovered_recoil_ /
                                 n_fieldmap_ecal_extrap_failed_recoil_
                           : 0.0)
                   << "%)";
  }
}

void CKFProcessor::configure(framework::config::Parameters& parameters) {
  dumpobj_ = parameters.get<bool>("dumpobj", 0);
  pionstates_ = parameters.get<int>("pionstates", 0);

  bfield_ = parameters.get<double>("bfield", -1.5);
  const_b_field_ = parameters.get<bool>("const_b_field", false);
  field_map_ = parameters.get<std::string>("field_map");
  propagator_step_size_ = parameters.get<double>("propagator_step_size", 200.);
  propagator_max_steps_ = parameters.get<int>("propagator_max_steps", 10000);
  measurement_collection_ = parameters.get<std::string>(
      "measurement_collection", "TaggerMeasurements");
  outlier_pval_ = parameters.get<double>("outlier_pval_", 3.84);

  debug_acts_ = parameters.get<bool>("debug_acts", false);

  remove_stereo_ = parameters.get<bool>("remove_stereo", false);
  use1_dmeasurements_ = parameters.get<bool>("use1Dmeasurements", true);
  min_hits_ = parameters.get<int>("min_hits", 7);

  // Ckf specific options
  use_extrapolate_location_ =
      parameters.get<bool>("use_extrapolate_location", true);
  extrapolate_location_ =
      parameters.get<std::vector<double>>("extrapolate_location", {0., 0., 0.});
  use_seed_perigee_ = parameters.get<bool>("use_seed_perigee", false);

  // seeds from the event
  seed_coll_name_ = parameters.get<std::string>("seed_coll_name", "seedTracks");

  sim_particles_coll_name_ =
      parameters.get<std::string>("sim_particles_coll_name");
  sim_particles_event_passname_ =
      parameters.get<std::string>("sim_particles_event_passname");

  // output track collection
  out_trk_collection_ =
      parameters.get<std::string>("out_trk_collection", "Tracks");

  // keep track on which system tracking is running
  tagger_tracking_ = parameters.get<bool>("tagger_tracking", true);

  // BField Systematics
  map_offset_ =
      parameters.get<std::vector<double>>("map_offset_", {0., 0., 0.});

  input_pass_name_ = parameters.get<std::string>("input_pass_name");
}  // end of configure()

auto CKFProcessor::makeGeoIdSourceLinkMap(
    const geo::TrackersTrackingGeometry& tg,
    const std::vector<ldmx::Measurement>& measurements)
    -> std::unordered_multimap<Acts::GeometryIdentifier,
                               acts_examples::IndexSourceLink> {
  std::unordered_multimap<Acts::GeometryIdentifier,
                          acts_examples::IndexSourceLink>
      geo_id_sl_map;

  ldmx_log(debug) << "The makeGeoIdSourceLinkMap has " << measurements.size()
                  << " measurements";

  // Check the hits associated to the surfaces
  for (unsigned int i_meas = 0; i_meas < measurements.size(); i_meas++) {
    ldmx::Measurement meas = measurements.at(i_meas);
    unsigned int layerid = meas.getLayerID();

    const Acts::Surface* hit_surface = tg.getSurface(layerid);

    if (hit_surface) {
      // Transform the ldmx space point from global to local and store the
      // information

      acts_examples::IndexSourceLink idx_sl(hit_surface->geometryId(), i_meas);
      // mg aug 2024 ... these don't print statements
      // don't compile using v36 in Acts...figure out later
      /*
      ldmx_log(debug)
          << "Insert measurement on surface located at::"
          << hit_surface->transform(geometry_context()).translation();
      ldmx_log(debug) << "and geoId::" << hit_surface->geometryId();

      ldmx_log(debug) << "Surface info::"
                      << std::tie(*hit_surface, geometry_context());
      */
      geo_id_sl_map.insert(std::make_pair(hit_surface->geometryId(), idx_sl));

    } else
      ldmx_log(debug) << getName() << "::HIT " << i_meas << " at layer_"
                      << (measurements.at(i_meas)).getLayerID()
                      << " is not associated to any surface?!";
  }

  return geo_id_sl_map;
}

template <typename geometry_t, typename source_link_hash_t,
          typename source_link_equality_t>
std::vector<std::vector<std::size_t>> CKFProcessor::computeSharedHits(
    std::vector<ldmx::Track> tracks, std::vector<ldmx::Measurement> meas_coll,
    geometry_t& tg, source_link_hash_t&& sourceLinkHash,
    source_link_equality_t&& sourceLinkEquality) const {
  auto measurement_index_map =
      std::unordered_map<Acts::SourceLink, std::size_t, source_link_hash_t,
                         source_link_equality_t>(0, sourceLinkHash,
                                                 sourceLinkEquality);

  std::vector<std::vector<std::size_t>> measurements_per_track;
  boost::container::flat_map<std::size_t,
                             boost::container::flat_set<std::size_t>>
      tracks_per_measurement;
  std::vector<std::size_t> shared_measurements_per_track;
  auto number_of_tracks = 0;

  // Iterate through all input tracks, collect their properties like measurement
  // count and chi2 and fill the measurement map in order to relate tracks to
  // each other if they have shared hits.
  for (const auto& track : tracks) {
    // Kick out tracks that do not fulfill our initial requirements
    // if (track.getNhits() < n_measurements_min_) {
    //   continue;
    // }

    std::vector<std::size_t> measurements;
    for (auto imeas : track.getMeasurementsIdxs()) {
      auto meas = meas_coll.at(imeas);
      const Acts::Surface* hit_surface = tg.getSurface(meas.getLayerID());
      // Store the index_ source link
      acts_examples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
      Acts::SourceLink source_link = Acts::SourceLink(idx_sl);

      auto emplace = measurement_index_map.try_emplace(
          source_link, measurement_index_map.size());
      measurements.push_back(emplace.first->second);
    }

    measurements_per_track.push_back(std::move(measurements));

    ++number_of_tracks;
  }

  // Now we relate measurements to tracks
  for (std::size_t i_track = 0; i_track < number_of_tracks; ++i_track) {
    for (auto i_measurement : measurements_per_track[i_track]) {
      tracks_per_measurement[i_measurement].insert(i_track);
    }
  }

  // Finally, we can accumulate the number of shared measurements per track
  shared_measurements_per_track = std::vector<std::size_t>(number_of_tracks, 0);

  std::vector<std::vector<std::size_t>> shared_measurement_idxs_per_track;
  for (std::size_t i_track = 0; i_track < number_of_tracks; ++i_track) {
    std::vector<std::size_t> shared_measurement_idxs;
    for (auto i_measurement : measurements_per_track[i_track]) {
      if (tracks_per_measurement[i_measurement].size() > 1) {
        ++shared_measurements_per_track[i_track];
        shared_measurement_idxs.push_back(i_measurement);
      }
    }
    shared_measurement_idxs_per_track.push_back(shared_measurement_idxs);
  }
  return shared_measurement_idxs_per_track;
}

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::CKFProcessor)
