#include "Tracking/Reco/GSFProcessor.h"

#include <algorithm>

#include "Acts/EventData/SourceLink.hpp"

namespace tracking {
namespace reco {

GSFProcessor::GSFProcessor(const std::string& name, framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void GSFProcessor::onNewRun(const ldmx::RunHeader& rh) {
  // Custom transformation of the interpolated bfield map
  bool debug_transform = false;
  auto transform_pos = [debug_transform](const Acts::Vector3& pos_) {
    Acts::Vector3 rot_pos;
    rot_pos(0) = pos_(1);
    rot_pos(1) = pos_(2);
    rot_pos(2) = pos_(0) + DIPOLE_OFFSET;

    // Apply A rotation around the center of the magnet. (I guess offset first
    // and then rotation)

    if (debug_transform) {
      std::cout << "PF::DEFAULT3 TRANSFORM" << std::endl;
      std::cout << "PF::Check:: transforming Pos" << std::endl;
      std::cout << pos_ << std::endl;
      std::cout << "TO" << std::endl;
      std::cout << rot_pos << std::endl;
    }

    return rot_pos;
  };

  Acts::RotationMatrix3 rotation = Acts::RotationMatrix3::Identity();
  double scale = 1.;

  auto transform_b_field = [rotation, scale, debug_transform](
                               const Acts::Vector3& field,
                               const Acts::Vector3& /*pos_*/) {
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

    if (debug_transform) {
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
                        transform_pos, transform_b_field));

  auto acts_logging_level = Acts::Logging::ERROR;

  if (debug_) acts_logging_level = Acts::Logging::VERBOSE;

  // Setup the GSF Fitter

  // Stepper
  // Acts::MixtureReductionMethod finalReductionMethod;
  // const auto multi_stepper = Acts::MultiEigenStepperLoop{map};

  // Acts::ComponentMergeMethod reductionMethod =
  //    Acts::ComponentMergeMethod::eMaxWeight;
  //  Acts::MultiEigenStepperLoop multi_stepper(
  //      map, reductionMethod,
  //      Acts::getDefaultLogger("GSF_STEP", acts_loggingLevel));

  Acts::MultiEigenStepperLoop multi_stepper(map);
  // Detailed Stepper

  // Acts::MultiEigenStepperLoop multi_stepper(map, finalReductionMethod);

  // Navigator
  Acts::Navigator::Config nav_cfg{geometry().getTG()};
  nav_cfg.resolveMaterial = true;
  nav_cfg.resolvePassive = false;
  nav_cfg.resolveSensitive = true;
  const Acts::Navigator navigator(nav_cfg);

  auto gsf_propagator =
      GsfPropagator(std::move(multi_stepper), std::move(navigator),
                    Acts::getDefaultLogger("GSF_PROP", acts_logging_level));

  BetheHeitlerApprox bethe_heitler = Acts::makeDefaultBetheHeitlerApprox();

  const auto gsf_logger = Acts::getDefaultLogger("GSF", acts_logging_level);

  gsf_ = std::make_unique<std::decay_t<decltype(*gsf_)>>(
      std::move(gsf_propagator), std::move(bethe_heitler),
      Acts::getDefaultLogger("GSF", acts_logging_level));

  const auto stepper = Acts::EigenStepper<>{map};
  propagator_ = std::make_unique<Propagator>(
      stepper, navigator, Acts::getDefaultLogger("PROP", acts_logging_level));

  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(
      *propagator_, geometryContext(), magneticFieldContext());
}

void GSFProcessor::configure(framework::config::Parameters& parameters) {
  out_trk_collection_ =
      parameters.get<std::string>("out_trk_collection", "GSFTracks");

  track_collection_ =
      parameters.get<std::string>("trackCollection", "TaggerTracks");
  meas_collection_ =
      parameters.get<std::string>("measCollection", "DigiTaggerSimHits");

  track_passname_ = parameters.get<std::string>("track_passname");
  meas_passname_ = parameters.get<std::string>("meas_passname");
  track_collection_event_passname_ =
      parameters.get<std::string>("track_collection_event_passname");
  meas_collection_event_passname_ =
      parameters.get<std::string>("meas_collection_event_passname");

  max_components_ = parameters.get<int>("maxComponents", 4);
  abort_on_error_ = parameters.get<bool>("abortOnError", false);
  disable_all_material_handling_ =
      parameters.get<bool>("disableAllMaterialHandling", false);
  weight_cutoff_ = parameters.get<double>("weight_cutoff_", 1.0e-4);

  propagator_max_steps_ = parameters.get<int>("propagator_maxSteps", 10000);
  propagator_step_size_ = parameters.get<double>("propagator_step_size", 200.);
  field_map_ = parameters.get<std::string>("field_map");
  use_perigee_ = parameters.get<bool>("usePerigee", false);

  debug_ = parameters.get<bool>("debug", false);
  tagger_tracking_ = parameters.get<bool>("taggerTracking", true);

  // final_reduction_method_ =
  // parameters.get<double>("finalReductionMethod",);
}

void GSFProcessor::produce(framework::Event& event) {
  // General Setup

  auto tg{geometry()};

  // Retrieve the tracks
  if (!event.exists(track_collection_, track_collection_event_passname_))
    return;
  auto tracks{
      event.getCollection<ldmx::Track>(track_collection_, track_passname_)};

  // Retrieve the measurements
  if (!event.exists(meas_collection_, meas_collection_event_passname_)) return;
  auto measurements{
      event.getCollection<ldmx::Measurement>(meas_collection_, meas_passname_)};

  tracking::sim::LdmxMeasurementCalibrator calibrator{measurements};

  // GSF Setup
  Acts::GainMatrixUpdater updater;
  Acts::GsfExtensions<Acts::VectorMultiTrajectory> gsf_extensions;
  gsf_extensions.updater.connect<
      &Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(
      &updater);
  gsf_extensions.calibrator
      .connect<&tracking::sim::LdmxMeasurementCalibrator::calibrate1d<
          Acts::VectorMultiTrajectory>>(&calibrator);

  // Surface Accessor
  struct SurfaceAccessor {
    const Acts::TrackingGeometry* tracking_geometry_;

    const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const {
      const auto& index_source_link =
          sourceLink.get<acts_examples::IndexSourceLink>();
      return tracking_geometry_->findSurface(index_source_link.geometryId());
    }
  };

  SurfaceAccessor m_sl_surface_accessor{tg.getTG().get()};
  // m_slSurfaceAccessor.trackingGeometry = tg.getTG();
  gsf_extensions.surfaceAccessor.connect<&SurfaceAccessor::operator()>(
      &m_sl_surface_accessor);
  gsf_extensions.mixtureReducer.connect<&Acts::reduceMixtureLargestWeights>();

  // Propagator Options

  // Move this at the start of the producer
  Acts::PropagatorOptions<Acts::StepperPlainOptions,
                          Acts::NavigatorPlainOptions, ActionList, AbortList>
      propagator_options(geometryContext(), magneticFieldContext());

  propagator_options.pathLimit = std::numeric_limits<double>::max();

  // Activate loop protection at some pt value
  propagator_options.loopProtection =
      false;  //(startParameters.transverseMomentum() < cfg.ptLoopers);

  // Switch the material interaction on/off & eventually into logging mode
  auto& m_interactor =
      propagator_options.actionList.get<Acts::MaterialInteractor>();
  m_interactor.multipleScattering = true;
  m_interactor.energyLoss = true;
  m_interactor.recordInteractions = false;

  // The logger can be switched to sterile, e.g. for timing logging
  auto& s_logger =
      propagator_options.actionList.get<Acts::detail::SteppingLogger>();
  s_logger.sterile = true;
  // Set a maximum step size
  propagator_options.stepping.maxStepSize =
      propagator_step_size_ * Acts::UnitConstants::mm;
  propagator_options.maxSteps = propagator_max_steps_;

  // Electron hypothesis
  //  propagator_options.mass = 0.511 * Acts::UnitConstants::MeV;

  // GSF Options, to be moved out of produce loop

  std::shared_ptr<const Acts::PerigeeSurface> origin_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(0., 0., 0.));

  std::shared_ptr<const Acts::PerigeeSurface> tagger_layer_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(-700., 0., 0.));

  std::shared_ptr<const Acts::PerigeeSurface> reference_surface =
      origin_surface;
  if (tagger_tracking_) {
    reference_surface = tagger_layer_surface;
  }

  /*
  Acts::GsfOptions<Acts::VectorMultiTrajectory> gsfOptions{
      geometry_context(),    magnetic_field_context(),
      calibration_context(), gsf_extensions,
      propagator_options,    &(*origin_surface),
      max_components_,        weight_cutoff_,
      abort_on_error_,         disable_all_material_handling_};
  */
  Acts::GsfOptions<Acts::VectorMultiTrajectory> gsf_options{
      geometryContext(), magneticFieldContext(), calibrationContext()};

  gsf_options.extensions = gsf_extensions;
  gsf_options.propagatorPlainOptions = propagator_options;
  gsf_options.referenceSurface = &(*reference_surface);
  gsf_options.maxComponents = max_components_;
  gsf_options.weightCutoff = weight_cutoff_;
  gsf_options.abortOnError = abort_on_error_;
  gsf_options.disableAllMaterialHandling = disable_all_material_handling_;

  // Output track container
  std::vector<ldmx::Track> out_tracks;

  // Acts containers
  Acts::VectorTrackContainer vtc;
  Acts::VectorMultiTrajectory mtj;
  Acts::TrackContainer tc{vtc, mtj};

  // Loop on tracks
  unsigned int itrk = 0;

  for (auto& track : tracks) {
    // Retrieve measurements on track
    std::vector<ldmx::Measurement> meas_on_track;

    // std::vector<ActsExamples::IndexSourceLink> fit_trackSourceLinks;
    std::vector<Acts::SourceLink> fit_track_source_links;

    for (auto imeas : track.getMeasurementsIdxs()) {
      auto meas = measurements.at(imeas);
      meas_on_track.push_back(meas);

      // Retrieve the surface

      const Acts::Surface* hit_surface =
          tg.geo::TrackingGeometry::getSurface(meas.getLayerID());

      // Store the index_ source link
      acts_examples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
      fit_track_source_links.push_back(Acts::SourceLink(idx_sl));
    }

    // Reverse the order of the vectors
    std::reverse(meas_on_track.begin(), meas_on_track.end());
    std::reverse(fit_track_source_links.begin(), fit_track_source_links.end());

    for (auto m : meas_on_track) {
      ldmx_log(debug) << "Measurement:\n" << m << "\n";
    }

    ldmx_log(debug) << "GSF Refitting";

    // Get the track parameters

    std::shared_ptr<Acts::PerigeeSurface> perigee =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
            track.getPerigeeX(), track.getPerigeeY(), track.getPerigeeZ()));

    Acts::BoundTrackParameters trk_btp =
        tracking::sim::utils::boundTrackParameters(track, perigee);

    std::shared_ptr<Acts::Surface> beam_origin_surface =
        tracking::sim::utils::unboundSurface(-700);

    const std::shared_ptr<Acts::Surface> target_surface =
        tracking::sim::utils::unboundSurface(0.);

    const std::shared_ptr<Acts::Surface> ecal_surface =
        tracking::sim::utils::unboundSurface(240.5);

    Acts::BoundTrackParameters trk_btp_b_o =
        tracking::sim::utils::boundTrackParameters(track, perigee);

    if (tagger_tracking_) {
      if (!track.getTrackState(ldmx::TrackStateType::AtBeamOrigin)
               .has_value()) {
        ldmx_log(warn) << "Failed retreiving AtBeamOrigin TrackState for "
                          "track. Skipping..";
        continue;
      }

      auto ts = track.getTrackState(ldmx::TrackStateType::AtBeamOrigin).value();
      trk_btp_b_o = tracking::sim::utils::btp(
          ts, beam_origin_surface,
          11);  // 11 == electron PDGid...hardcode for now
    } else {
      if (!track.getTrackState(ldmx::TrackStateType::AtTarget).has_value()) {
        ldmx_log(warn)
            << "Failed retreiving AtTarget TrackState for track. Skipping..";
        continue;
      }
      auto ts = track.getTrackState(ldmx::TrackStateType::AtTarget).value();
      trk_btp_b_o = tracking::sim::utils::btp(ts, target_surface, 11);
    }
    const Acts::BoundVector& trkpars = trk_btp.parameters();
    ldmx_log(debug) << "CKF Track parameters" << std::endl
                    << trkpars[0] << " " << trkpars[1] << " " << trkpars[2]
                    << " " << trkpars[3] << " " << trkpars[4] << " "
                    << trkpars[5] << std::endl
                    << "Perigee Surface" << std::endl
                    << track.getPerigeeX() << " " << track.getPerigeeY() << " "
                    << track.getPerigeeZ();

    Acts::Vector3 trk_pos = trk_btp.position(geometryContext());

    ldmx_log(debug) << trk_pos(0) << " " << trk_pos(1) << " " << trk_pos(2)
                    << std::endl;

    const Acts::BoundVector& trkpars_b_o = trk_btp_b_o.parameters();
    ldmx_log(debug) << "CKF BeamOrigin track parameters" << std::endl
                    << trkpars_b_o[0] << " " << trkpars_b_o[1] << " "
                    << trkpars_b_o[2] << " " << trkpars_b_o[3] << " "
                    << trkpars_b_o[4] << " " << trkpars_b_o[5] << " ";

    Acts::Vector3 trk_pos_b_o = trk_btp_b_o.position(geometryContext());
    ldmx_log(debug) << trk_pos_b_o(0) << " " << trk_pos_b_o(1) << " "
                    << trk_pos_b_o(2) << std::endl;

    auto gsf_refit_result =
        gsf_->fit(fit_track_source_links.begin(), fit_track_source_links.end(),
                  trk_btp_b_o, gsf_options, tc);

    if (!gsf_refit_result.ok()) {
      ldmx_log(warn) << "GSF re-fit failed" << std::endl;
      continue;
    }

    if (tc.size() < 1) continue;

    auto gsftrk = tc.getTrack(itrk);
    calculateTrackQuantities(gsftrk);

    const Acts::BoundVector& perigee_pars = gsftrk.parameters();
    const Acts::BoundMatrix& trk_cov = gsftrk.covariance();
    const Acts::Surface& perigee_surface = gsftrk.referenceSurface();

    ldmx_log(debug)
        << "Found track:" << std::endl
        << "Track states " << gsftrk.nTrackStates() << std::endl
        << perigee_pars[Acts::eBoundLoc0] << " "
        << perigee_pars[Acts::eBoundLoc1] << " "
        << perigee_pars[Acts::eBoundPhi] << " "
        << perigee_pars[Acts::eBoundTheta] << " "
        << perigee_pars[Acts::eBoundQOverP] << std::endl
        << "Reference Surface" << std::endl
        << " " << perigee_surface.transform(geometryContext()).translation()(0)
        << " " << perigee_surface.transform(geometryContext()).translation()(1)
        << " " << perigee_surface.transform(geometryContext()).translation()(2)
        << std::endl;

    ldmx::Track trk = ldmx::Track();

    bool success = false;
    if (tagger_tracking_) {
      ldmx_log(debug) << "Target extrapolation";
      ldmx::Track::TrackState ts_at_target;

      success = trk_extrap_->trackStateAtSurface(
          gsftrk, target_surface, ts_at_target, ldmx::TrackStateType::AtTarget);

      if (success) trk.addTrackState(ts_at_target);
    } else {
      ldmx_log(debug) << "Ecal Extrapolation";
      ldmx::Track::TrackState ts_at_ecal;
      success = trk_extrap_->trackStateAtSurface(
          gsftrk, ecal_surface, ts_at_ecal, ldmx::TrackStateType::AtECAL);

      if (success) trk.addTrackState(ts_at_ecal);
    }

    trk.setPerigeeLocation(
        perigee_surface.transform(geometryContext()).translation()(0),
        perigee_surface.transform(geometryContext()).translation()(1),
        perigee_surface.transform(geometryContext()).translation()(2));

    trk.setChi2(gsftrk.chi2());
    trk.setNhits(gsftrk.nMeasurements());
    trk.setNdf(gsftrk.nMeasurements() - 5);
    trk.setPerigeeParameters(
        tracking::sim::utils::convertActsToLdmxPars(perigee_pars));
    std::vector<double> v_trk_cov;
    tracking::sim::utils::flatCov(trk_cov, v_trk_cov);
    trk.setPerigeeCov(v_trk_cov);
    Acts::Vector3 trk_momentum = gsftrk.momentum();
    auto trk_mom_gbl = tracking::sim::utils::acts2LdmxMomentum(trk_momentum);
    trk.setMomentum(trk_mom_gbl[0], trk_mom_gbl[1], trk_mom_gbl[2]);

    // truth information
    trk.setTrackID(track.getTrackID());
    trk.setPdgID(track.getPdgID());
    trk.setTruthProb(track.getTruthProb());

    itrk++;

    out_tracks.push_back(trk);

  }  // loop on tracks

  event.add(out_trk_collection_, out_tracks);
}

void GSFProcessor::onProcessStart() {};
void GSFProcessor::onProcessEnd() {};

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::GSFProcessor)
