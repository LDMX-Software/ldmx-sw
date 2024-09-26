#include "Tracking/Reco/CKFProcessor.h"

#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "SimCore/Event/SimParticle.h"
#include "Tracking/Reco/TruthMatchingTool.h"
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

CKFProcessor::~CKFProcessor() {}

void CKFProcessor::onNewRun(const ldmx::RunHeader& rh) {
  profiling_map_["setup"] = 0.;
  profiling_map_["hits"] = 0.;
  profiling_map_["seeds"] = 0.;
  profiling_map_["ckf_setup"] = 0.;
  profiling_map_["ckf_run"] = 0.;
  profiling_map_["result_loop"] = 0.;

  // Generate a constant magnetic field
  Acts::Vector3 b_field(0., 0., bfield_ * Acts::UnitConstants::T);

  // Setup a constant magnetic field
  const auto constBField = std::make_shared<Acts::ConstantBField>(b_field);

  // Define the target surface - be careful:
  //  x - downstream
  //  y - left (when looking along x)
  //  z - up
  //  Passing identity here means that your target surface is oriented in the
  //  same way
  surf_rotation = Acts::RotationMatrix3::Zero();
  // u direction along +Y
  surf_rotation(1, 0) = 1;
  // v direction along +Z
  surf_rotation(2, 1) = 1;
  // w direction along +X
  surf_rotation(0, 2) = 1;

  Acts::Vector3 target_pos(0., 0., 0.);
  Acts::Translation3 target_translation(target_pos);
  Acts::Transform3 target_transform(target_translation * surf_rotation);

  // Unbounded surface
  target_surface =
      Acts::Surface::makeShared<Acts::PlaneSurface>(target_transform);

  // Custom transformation of the interpolated bfield map
  bool debugTransform = false;
  auto transformPos = [this, debugTransform](const Acts::Vector3& pos) {
    Acts::Vector3 rot_pos;
    rot_pos(0) = pos(1);
    rot_pos(1) = pos(2);
    rot_pos(2) = pos(0) + DIPOLE_OFFSET;

    // Systematic effect
    rot_pos(0) += this->map_offset_[0];
    rot_pos(1) += this->map_offset_[1];
    rot_pos(2) += this->map_offset_[2];

    // Apply A rotation around the center of the magnet. (I guess offset first
    // and then rotation)

    if (debugTransform) {
      std::cout << "PF::DEFAULT3 TRANSFORM" << std::endl;
      std::cout << "PF::Check:: transforming Pos" << std::endl;
      std::cout << pos << std::endl;
      std::cout << "TO" << std::endl;
      std::cout << rot_pos << std::endl;
    }

    return rot_pos;
  };

  Acts::RotationMatrix3 rotation = Acts::RotationMatrix3::Identity();
  double scale = 1.;

  auto transformBField = [rotation, scale, debugTransform](
                             const Acts::Vector3& field,
                             const Acts::Vector3& /*pos*/) {
    // Rotate the field in tracking coordinates
    Acts::Vector3 rot_field;
    rot_field(0) = field(2);
    rot_field(1) = field(0);
    rot_field(2) = field(1);

    // Scale the field
    rot_field = scale * rot_field;

    // Rotate the field
    rot_field = rotation * rot_field;

    // A distortion scaled by position.
    if (debugTransform) {
      std::cout << "PF::DEFAULT3 TRANSFORM" << std::endl;
      std::cout << "PF::Check:: transforming" << std::endl;
      std::cout << field << std::endl;
      std::cout << "TO" << std::endl;
      std::cout << rot_field << std::endl;
    }

    return rot_field;
  };

  // Setup a interpolated bfield map
  const auto map = std::make_shared<InterpolatedMagneticField3>(
      loadDefaultBField(field_map_,
                        // default_transformPos,
                        // default_transformBField));
                        transformPos, transformBField));

  auto acts_loggingLevel = Acts::Logging::FATAL;
  if (debug_acts_) acts_loggingLevel = Acts::Logging::VERBOSE;

  // Setup the steppers
  const auto stepper = Acts::EigenStepper<>{map};
  const auto const_stepper = Acts::EigenStepper<>{constBField};
  const auto multi_stepper = Acts::MultiEigenStepperLoop{map};

  // Setup the navigator
  Acts::Navigator::Config navCfg{geometry().getTG()};
  navCfg.resolveMaterial = true;
  navCfg.resolvePassive = true;
  navCfg.resolveSensitive = true;
  const Acts::Navigator navigator(navCfg);

  // Setup the propagators
  propagator_ =
      const_b_field_
          ? std::make_unique<CkfPropagator>(const_stepper, navigator)
          : std::make_unique<CkfPropagator>(
                stepper, navigator,
                Acts::getDefaultLogger("ACTS_PROP", acts_loggingLevel));

  // Setup the finder / fitters
  ckf_ = std::make_unique<std::decay_t<decltype(*ckf_)>>(
      *propagator_, Acts::getDefaultLogger("CKF", acts_loggingLevel));
  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(
      *propagator_, geometry_context(), magnetic_field_context());
}

void CKFProcessor::produce(framework::Event& event) {
  eventnr_++;
  // get the tracking geometry from conditions
  auto tg{geometry()};

  // TODO use global variable instead and call clear;

  std::vector<ldmx::Track> tracks;

  auto start = std::chrono::high_resolution_clock::now();

  nevents_++;
  if (nevents_ % 1000 == 0) ldmx_log(info) << "events processed:" << nevents_;

  auto loggingLevel = Acts::Logging::DEBUG;
  ACTS_LOCAL_LOGGER(
      Acts::getDefaultLogger("LDMX Tracking Geometry Maker", loggingLevel));

  // Move this at the start of the producer
  Acts::PropagatorOptions<Acts::StepperPlainOptions,
                          Acts::NavigatorPlainOptions, ActionList, AbortList>
      propagator_options(geometry_context(), magnetic_field_context());

  propagator_options.pathLimit = std::numeric_limits<double>::max();
  // Activate loop protection at some pt value
  propagator_options.loopProtection =
      false;  //(startParameters.transverseMomentum() < cfg.ptLoopers);

  // Switch the material interaction on/off & eventually into logging mode
  auto& mInteractor =
      propagator_options.actionList.get<Acts::MaterialInteractor>();
  mInteractor.multipleScattering = true;
  mInteractor.energyLoss = true;
  mInteractor.recordInteractions = false;

  // The logger can be switched to sterile, e.g. for timing logging
  auto& sLogger =
      propagator_options.actionList.get<Acts::detail::SteppingLogger>();
  sLogger.sterile = true;
  // Set a maximum step size
  propagator_options.stepping.maxStepSize =
      propagator_step_size_ * Acts::UnitConstants::mm;
  propagator_options.maxSteps = propagator_maxSteps_;

  // #######################//
  // Kalman Filter algorithm//
  // #######################//

  // Step 1 - Form the source links

  // a) Loop over the sim Hits

  auto setup = std::chrono::high_resolution_clock::now();
  profiling_map_["setup"] +=
      std::chrono::duration<double, std::milli>(setup - start).count();

  const std::vector<ldmx::Measurement> measurements =
      event.getCollection<ldmx::Measurement>(measurement_collection_);

  // check if SimParticleMap is available for truth matching
  std::shared_ptr<tracking::sim::TruthMatchingTool> truthMatchingTool = nullptr;
  std::map<int, ldmx::SimParticle> particleMap;

  if (event.exists("SimParticles")) {
    ldmx_log(debug) << "Setting up track truth matching tool";
    particleMap = event.getMap<int, ldmx::SimParticle>("SimParticles");
    truthMatchingTool = std::make_shared<tracking::sim::TruthMatchingTool>(
        particleMap, measurements);
  }

  // The mapping between the geometry identifier
  // and the IndexsourceLink that points to the hit
  const auto geoId_sl_map = makeGeoIdSourceLinkMap(tg, measurements);

  auto hits = std::chrono::high_resolution_clock::now();
  profiling_map_["hits"] +=
      std::chrono::duration<double, std::milli>(hits - setup).count();

  // ============   Setup the CKF  ============

  // Retrieve the seeds

  ldmx_log(debug) << "Retrieve the seeds::" << seed_coll_name_;

  const std::vector<ldmx::Track> seed_tracks =
      event.getCollection<ldmx::Track>(seed_coll_name_);

  ldmx_log(debug) << "Number of seeds::" << seed_tracks.size();

  // Run the CKF on each seed and produce a track candidate
  std::vector<Acts::BoundTrackParameters> startParameters;

  // Tune the reconstruction for different PDG ID
  std::vector<int> seedPDGID;

  for (auto& seed : seed_tracks) {
    // Transform the seed track to bound parameters
    std::shared_ptr<Acts::PerigeeSurface> perigeeSurface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
            seed.getPerigeeX(), seed.getPerigeeY(), seed.getPerigeeZ()));

    Acts::BoundVector paramVec;
    paramVec << seed.getD0(), seed.getZ0(), seed.getPhi(), seed.getTheta(),
        seed.getQoP(), seed.getT();

    Acts::BoundSquareMatrix covMat =
        tracking::sim::utils::unpackCov(seed.getPerigeeCov());

    ldmx_log(debug) << "perigee" << std::endl
                    << seed.getPerigeeX() << " " << seed.getPerigeeY() << " "
                    << seed.getPerigeeZ() << std::endl
                    << "start Parameters" << std::endl
                    << paramVec;

    ldmx_log(debug) << "cov matrix" << std::endl << covMat << std::endl;

    // need to set particle hypothesis...set to electron for now...
    auto partHypo{Acts::SinglyChargedParticleHypothesis::electron()};
    startParameters.push_back(
        Acts::BoundTrackParameters(perigeeSurface, paramVec, covMat, partHypo));

    seedPDGID.push_back(seed.getPdgID());

    nseeds_++;
  }  // loop on seeds

  if (startParameters.size() < 1) {
    std::vector<ldmx::Track> empty;
    event.add(out_trk_collection_, empty);
    return;
  }

  auto seeds = std::chrono::high_resolution_clock::now();
  profiling_map_["seeds"] +=
      std::chrono::duration<double, std::milli>(seeds - hits).count();

  Acts::GainMatrixUpdater kfUpdater;

  // configuration for the measurement selector. Empty geometry identifier means
  // applicable to all the detector elements

  Acts::MeasurementSelector::Config measurementSelectorCfg = {
      // global default: no chi2 cut, only one measurement per surface
      {Acts::GeometryIdentifier(), {{}, {outlier_pval_}, {1u}}},
  };

  Acts::MeasurementSelector measSel{measurementSelectorCfg};

  tracking::sim::LdmxMeasurementCalibrator calibrator{measurements};

  Acts::CombinatorialKalmanFilterExtensions<TrackContainer> ckf_extensions;

  if (use1Dmeasurements_)
    ckf_extensions.calibrator
        .connect<&tracking::sim::LdmxMeasurementCalibrator::calibrate_1d<
            Acts::VectorMultiTrajectory>>(&calibrator);

  else
    ckf_extensions.calibrator
        .connect<&tracking::sim::LdmxMeasurementCalibrator::calibrate<
            Acts::VectorMultiTrajectory>>(&calibrator);

  ckf_extensions.updater.connect<
      &Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(
      &kfUpdater);

  ckf_extensions.measurementSelector
      .connect<&Acts::MeasurementSelector::select<Acts::VectorMultiTrajectory>>(
          &measSel);

  ldmx_log(debug) << "SourceLinkAccessor..." << std::endl;

  // Create source link accessor and connect delegate
  struct SourceLinkAccIt {
    using BaseIt = decltype(geoId_sl_map.begin());
    BaseIt it;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

    using difference_type = typename BaseIt::difference_type;
    using iterator_category = typename BaseIt::iterator_category;
    // using value_type = typename BaseIt::value_type::second_type;
    using value_type = Acts::SourceLink;
    using pointer = typename BaseIt::pointer;
    using reference = value_type&;
#pragma GCC diagnostic pop

    SourceLinkAccIt& operator++() {
      ++it;
      return *this;
    }
    bool operator==(const SourceLinkAccIt& other) const {
      return it == other.it;
    }
    bool operator!=(const SourceLinkAccIt& other) const {
      return !(*this == other);
    }
    // const value_type& operator*() const { return it->second; }

    // by value
    value_type operator*() const { return value_type{it->second}; }
  };

  auto sourceLinkAccessor = [&](const Acts::Surface& surface)
      -> std::pair<SourceLinkAccIt, SourceLinkAccIt> {
    auto [begin, end] = geoId_sl_map.equal_range(surface.geometryId());
    return {SourceLinkAccIt{begin}, SourceLinkAccIt{end}};
  };

  Acts::SourceLinkAccessorDelegate<SourceLinkAccIt> sourceLinkAccessorDelegate;
  sourceLinkAccessorDelegate.connect<&decltype(sourceLinkAccessor)::operator(),
                                     decltype(sourceLinkAccessor)>(
      &sourceLinkAccessor);

  ldmx_log(debug) << "Surfaces..." << std::endl;

  std::shared_ptr<const Acts::PerigeeSurface> origin_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(0., 0., 0.));

  ldmx_log(debug) << "About to run CKF..." << std::endl;

  // run the CKF for all initial track states
  auto ckf_setup = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_setup"] +=
      std::chrono::duration<double, std::milli>(ckf_setup - seeds).count();

  auto ckf_run = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_run"] +=
      std::chrono::duration<double, std::milli>(ckf_run - ckf_setup).count();

  Acts::VectorTrackContainer vtc;
  Acts::VectorMultiTrajectory mtj;
  Acts::TrackContainer tc{vtc, mtj};

  for (size_t trackId = 0u; trackId < startParameters.size(); ++trackId) {
    // The seed has a track PdgID associated
    if (seedPDGID.at(trackId) != 0) {
      // int pdgID = seedPDGID.at(trackId);
    }

    // Define the CKF options here:
    const Acts::CombinatorialKalmanFilterOptions<SourceLinkAccIt,
                                                 TrackContainer>
        ckfOptions(TrackingGeometryUser::geometry_context(),
                   TrackingGeometryUser::magnetic_field_context(),
                   TrackingGeometryUser::calibration_context(),
                   sourceLinkAccessorDelegate, ckf_extensions,
                   propagator_options, true /* multiple scattering */,
                   false /* energy loss */);

    ldmx_log(debug) << "Running CKF on seed params "
                    << startParameters.at(trackId).parameters().transpose()
                    << std::endl;
    ldmx_log(debug) << "Checking options:  multiple scattering = "
                    << ckfOptions.multipleScattering
                    << "  energy loss = " << ckfOptions.energyLoss;
    auto results =
        ckf_->findTracks(startParameters.at(trackId), ckfOptions, tc);
    ldmx_log(debug) << "findTracks returned ... checking if ok";
    if (not results.ok()) {
      ldmx_log(debug) << "CKF Fit failed" << std::endl;
      continue;
    }

    // No track found
    // if (tc.size() < trackId + 1) continue;

    auto& tracksFromSeed = results.value();

    ldmx_log(debug) << "number of entries in results " << tracksFromSeed.size();
    for (auto& track : tracksFromSeed) {
      // do the track smoothing...this is not done in the CKF code anymore
      Acts::smoothTrack(geometry_context(), track);  // from TrackHelpers
      // make the empty ldmx::Track() and track state at target
      ldmx::Track trk = ldmx::Track();
      ldmx::Track::TrackState tsAtTarget;
      ldmx_log(debug) << "Found track: nMeas " << track.nMeasurements();
      ldmx_log(debug) << "Track states " << track.nTrackStates();
      ldmx_log(debug) << "chi2  " << track.chi2();

      for (const auto ts : track.trackStatesReversed()) {
        // Check TrackStates Quality
        ldmx_log(debug) << "Checking Track State at location "
                        << ts.referenceSurface()
                               .transform(geometry_context())
                               .translation()
                               .transpose()
                        << std::endl;

        ldmx_log(debug) << "Smoothed? " << ts.hasSmoothed() << std::endl;
        if (ts.hasSmoothed()) {
          ldmx_log(debug) << "Parameters \n"
                          << ts.smoothed().transpose() << std::endl;
          ldmx_log(debug) << "Covariance \n"
                          << ts.smoothedCovariance() << std::endl;
        }

        // Check if the track state is a measurement
        auto typeFlags = ts.typeFlags();

        if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag) &&
            ts.hasUncalibratedSourceLink()) {
          ldmx_log(debug) << " getting source link for this measurement";

          const ActsExamples::IndexSourceLink sl =
              ts.getUncalibratedSourceLink()
                  .template get<ActsExamples::IndexSourceLink>();

          ldmx_log(debug) << " looking up this index in measurements list";
          ldmx::Measurement ldmx_meas = measurements.at(sl.index());
          ldmx_log(debug) << "SourceLink Index::" << sl.index();
          ldmx_log(debug) << "Measurement:\n" << ldmx_meas << "\n";
          ldmx_log(debug) << " adding measurement to ldmx::track";
          trk.addMeasurementIndex(sl.index());
        }
      }
      bool success = trk_extrap_->TrackStateAtSurface(
          track, target_surface, tsAtTarget, ldmx::TrackStateType::AtTarget);
      ldmx_log(debug) << "target extrapolation success??? " << success;
      if (success) {
        ldmx_log(debug) << "Successfully obtained TS at target";
        ldmx_log(debug) << "Parameters At Target:  \n"
                        << tsAtTarget.params[0] << " " << tsAtTarget.params[1]
                        << " " << tsAtTarget.params[2] << " "
                        << tsAtTarget.params[3] << " " << tsAtTarget.params[4];

        trk.addTrackState(tsAtTarget);
      } else {
        ldmx_log(info)
            << "Could not extrapolate to target?  Printing track states:  ";
        ldmx_log(info) << "        nhits = " << track.nMeasurements();
        for (const auto ts : track.trackStatesReversed()) {
          ldmx_log(info) << "Smoothed? " << ts.hasSmoothed() << std::endl;
          if (ts.hasSmoothed()) {
            ldmx_log(info) << "momentum for track state = "
                           << 1 / ts.smoothed()[Acts::eBoundQOverP];
            ldmx_log(info) << "Parameters \n"
                           << ts.smoothed().transpose() << std::endl;
          } else {
            ldmx_log(info) << "Track state not smoothed?";
          }
        }
        ldmx_log(info) << "...skipping this track...";
        continue;
      }

      // get the BoundTrackParameters at the target
      // ...use to fill in the Acts::TrackProxy object
      // This isn't really necessary, since we can take
      // most everything for making the ldmx::track
      // from tsAtTarget...maybe useful for something?
      // -->one thing this does is allow Acts to
      // calculate the momentum 3-vector for you
      Acts::BoundTrackParameters boundStateAtTarget =
          tracking::sim::utils::btp(tsAtTarget, target_surface, 11);
      track.setReferenceSurface(target_surface);
      track.parameters() = boundStateAtTarget.parameters();

      ldmx_log(debug) << typeid(track).name();
      // These are the parameters at the target surface
      const Acts::BoundVector& track_pars = track.parameters();
      // const Acts::BoundMatrix& trk_cov = track.covariance();
      const Acts::Surface& track_surface = track.referenceSurface();
      ldmx_log(debug) << "Got the parameters, covariance, and perigee surface";

      ldmx_log(debug) << track_pars[Acts::eBoundLoc0];
      ldmx_log(debug) << track_pars[Acts::eBoundLoc1];
      ldmx_log(debug) << track_pars[Acts::eBoundTheta];
      ldmx_log(debug) << track_pars[Acts::eBoundPhi];
      ldmx_log(debug)
          << "Reference Surface" << std::endl
          << " " << track_surface.transform(geometry_context()).translation()(0)
          << " " << track_surface.transform(geometry_context()).translation()(1)
          << " "
          << track_surface.transform(geometry_context()).translation()(2);

      trk.setPerigeeLocation(
          0, 0, 0);  // the target...it's not really perigee anymore.
      trk.setPerigeeParameters(tsAtTarget.params);
      trk.setPerigeeCov(tsAtTarget.cov);

      ldmx_log(debug) << "setting chi2 and nHits:  " << track.chi2() << "    "
                      << track.nMeasurements();
      trk.setChi2(track.chi2());
      trk.setNhits(track.nMeasurements());
      // trk.setNdf(track.nDoF());
      // TODO Switch back to nDoF when Acts is fixed.
      trk.setNdf(track.nMeasurements() - 5);
      trk.setNsharedHits(track.nSharedHits());

      ldmx_log(debug) << "setting track momentum:  " << track.momentum();
      trk.setMomentum(track.momentum()[0], track.momentum()[1],
                      track.momentum()[2]);

      ldmx_log(debug) << "starting extrapolations";
      // Extrapolations

      const double ECAL_SCORING_PLANE = 240.5;
      Acts::Vector3 pos(ECAL_SCORING_PLANE, 0., 0.);
      Acts::Translation3 surf_translation(pos);
      Acts::Transform3 surf_transform(surf_translation * surf_rotation);

      // Unbounded surface
      const std::shared_ptr<Acts::PlaneSurface> ecal_surface =
          Acts::Surface::makeShared<Acts::PlaneSurface>(surf_transform);

      // Beam Origin unbounded surface
      const std::shared_ptr<Acts::Surface> beamOrigin_surface =
          tracking::sim::utils::unboundSurface(-700);

      if (taggerTracking_) {
        ldmx_log(debug) << "Beam Origin Extrapolation";
        ldmx::Track::TrackState tsAtBeamOrigin;
        success = trk_extrap_->TrackStateAtSurface(
            track, beamOrigin_surface, tsAtBeamOrigin,
            ldmx::TrackStateType::AtBeamOrigin);

        if (success) {
          trk.addTrackState(tsAtBeamOrigin);
          ldmx_log(debug) << "Successfully obtained TS at beam origin";
        }
      }

      // Recoil Extrapolation to ECAL only
      if (!taggerTracking_) {
        ldmx_log(debug) << "Ecal Extrapolation";
        ldmx::Track::TrackState tsAtEcal;
        success = trk_extrap_->TrackStateAtSurface(
            track, ecal_surface, tsAtEcal, ldmx::TrackStateType::AtECAL);

        if (success) {
          trk.addTrackState(tsAtEcal);
          ldmx_log(debug) << "Successfully obtained TS at Ecal";
          ldmx_log(debug) << "Parameters At Ecal:  \n"
                          << tsAtEcal.params[0] << " " << tsAtEcal.params[1]
                          << " " << tsAtEcal.params[2] << " "
                          << tsAtEcal.params[3] << " " << tsAtEcal.params[4];
        }
      }

      // Truth matching
      if (truthMatchingTool) {
        auto truthInfo = truthMatchingTool->TruthMatch(trk);
        trk.setTrackID(truthInfo.trackID);
        trk.setPdgID(truthInfo.pdgID);
        trk.setTruthProb(truthInfo.truthProb);
      }

      // At least min_hits_ hits and p > 50 MeV
      if (trk.getNhits() > min_hits_ && abs(1. / trk.getQoP()) > 0.05) {
        tracks.push_back(trk);
        ntracks_++;
      }
    }
  }  // loop seed track parameters

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
}

void CKFProcessor::onProcessStart() {
  if (use1Dmeasurements_)
    ldmx_log(info) << "use1Dmeasurements = " << std::boolalpha
                   << use1Dmeasurements_;
  if (remove_stereo_)
    ldmx_log(info) << "remove_stereo = " << std::boolalpha << remove_stereo_;
}

void CKFProcessor::onProcessEnd() {
  ldmx_log(info) << "found " << ntracks_ << " tracks  / " << nseeds_
                 << " nseeds";
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / nevents_ << " ms";
  ldmx_log(info) << "Breakdown::";
  ldmx_log(info) << "setup       Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["setup"] / nevents_
                 << " ms";
  ldmx_log(info) << "hits        Avg Time/Event = " << std::fixed
                 << std::setprecision(2) << profiling_map_["hits"] / nevents_
                 << " ms";
  ldmx_log(info) << "seeds       Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["seeds"] / nevents_
                 << " ms";
  ldmx_log(info) << "cf_setup    Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["ckf_setup"] / nevents_ << " ms";
  ldmx_log(info) << "ckf_run     Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["ckf_run"] / nevents_
                 << " ms";
  ldmx_log(info) << "result_loop Avg Time/Event = " << std::fixed
                 << std::setprecision(1)
                 << profiling_map_["result_loop"] / nevents_ << " ms";
}

void CKFProcessor::configure(framework::config::Parameters& parameters) {
  dumpobj_ = parameters.getParameter<bool>("dumpobj", 0);
  pionstates_ = parameters.getParameter<int>("pionstates", 0);

  bfield_ = parameters.getParameter<double>("bfield", -1.5);
  const_b_field_ = parameters.getParameter<bool>("const_b_field", false);
  field_map_ = parameters.getParameter<std::string>("field_map");
  propagator_step_size_ =
      parameters.getParameter<double>("propagator_step_size", 200.);
  propagator_maxSteps_ =
      parameters.getParameter<int>("propagator_maxSteps", 10000);
  measurement_collection_ = parameters.getParameter<std::string>(
      "measurement_collection", "TaggerMeasurements");
  outlier_pval_ = parameters.getParameter<double>("outlier_pval_", 3.84);

  debug_acts_ = parameters.getParameter<bool>("debug_acts", false);

  remove_stereo_ = parameters.getParameter<bool>("remove_stereo", false);
  use1Dmeasurements_ = parameters.getParameter<bool>("use1Dmeasurements", true);
  min_hits_ = parameters.getParameter<int>("min_hits", 7);

  // Ckf specific options
  use_extrapolate_location_ =
      parameters.getParameter<bool>("use_extrapolate_location", true);
  extrapolate_location_ = parameters.getParameter<std::vector<double>>(
      "extrapolate_location", {0., 0., 0.});
  use_seed_perigee_ = parameters.getParameter<bool>("use_seed_perigee", false);

  ldmx_log(debug) << " use_extrapolate_location ? "
                  << use_extrapolate_location_;
  ldmx_log(debug) << " use_seed_perigee ? " << use_seed_perigee_;

  // seeds from the event
  seed_coll_name_ =
      parameters.getParameter<std::string>("seed_coll_name", "seedTracks");

  // output track collection
  out_trk_collection_ =
      parameters.getParameter<std::string>("out_trk_collection", "Tracks");

  // keep track on which system tracking is running
  taggerTracking_ = parameters.getParameter<bool>("taggerTracking", true);

  // BField Systematics
  map_offset_ =
      parameters.getParameter<std::vector<double>>("map_offset_", {0., 0., 0.});
}

auto CKFProcessor::makeGeoIdSourceLinkMap(
    const geo::TrackersTrackingGeometry& tg,
    const std::vector<ldmx::Measurement>& measurements)
    -> std::unordered_multimap<Acts::GeometryIdentifier,
                               ActsExamples::IndexSourceLink> {
  std::unordered_multimap<Acts::GeometryIdentifier,
                          ActsExamples::IndexSourceLink>
      geoId_sl_map;

  ldmx_log(debug) << "makeGeoIdSourceLinkMap::Available measurements = "
                  << measurements.size();

  // Check the hits associated to the surfaces
  for (unsigned int i_meas = 0; i_meas < measurements.size(); i_meas++) {
    ldmx::Measurement meas = measurements.at(i_meas);
    unsigned int layerid = meas.getLayerID();

    const Acts::Surface* hit_surface = tg.getSurface(layerid);

    if (hit_surface) {
      // Transform the ldmx space point from global to local and store the
      // information

      ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(), i_meas);
      // mg aug 2024 ... these don't print statements
      // don't compile using v36 in Acts...figure out later
      /*
      ldmx_log(debug)
          << "Insert measurement on surface located at::"
          << hit_surface->transform(geometry_context()).translation();
      ldmx_log(debug) << "and geoId::" << hit_surface->geometryId()
                      << std::endl;

      ldmx_log(debug) << "Surface info::"
                      << std::tie(*hit_surface, geometry_context());
      */
      geoId_sl_map.insert(std::make_pair(hit_surface->geometryId(), idx_sl));

    } else
      std::cout << getName() << "::HIT " << i_meas << " at layer"
                << (measurements.at(i_meas)).getLayerID()
                << " is not associated to any surface?!" << std::endl;
  }

  return geoId_sl_map;
}

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, CKFProcessor)
