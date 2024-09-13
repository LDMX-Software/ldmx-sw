#include "Tracking/Reco/GSFProcessor.h"

#include <algorithm>

#include "Acts/EventData/SourceLink.hpp"

namespace tracking {
namespace reco {

GSFProcessor::GSFProcessor(const std::string& name, framework::Process& process)
    : TrackingGeometryUser(name, process) {}

GSFProcessor::~GSFProcessor() {}

void GSFProcessor::onNewRun(const ldmx::RunHeader& rh) {
  // Custom transformation of the interpolated bfield map
  bool debugTransform = false;
  auto transformPos = [this, debugTransform](const Acts::Vector3& pos) {
    Acts::Vector3 rot_pos;
    rot_pos(0) = pos(1);
    rot_pos(1) = pos(2);
    rot_pos(2) = pos(0) + DIPOLE_OFFSET;

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

  auto acts_loggingLevel = Acts::Logging::ERROR;

  if (debug_) acts_loggingLevel = Acts::Logging::VERBOSE;

  // Setup the GSF Fitter

  // Stepper
  // Acts::MixtureReductionMethod finalReductionMethod;
  // const auto multi_stepper = Acts::MultiEigenStepperLoop{map};

  Acts::ComponentMergeMethod reductionMethod =
      Acts::ComponentMergeMethod::eMaxWeight;
  //  Acts::MultiEigenStepperLoop multi_stepper(
  //      map, reductionMethod,
  //      Acts::getDefaultLogger("GSF_STEP", acts_loggingLevel));

  Acts::MultiEigenStepperLoop multi_stepper(map);
  // Detailed Stepper

  // Acts::MultiEigenStepperLoop multi_stepper(map, finalReductionMethod);

  // Navigator
  Acts::Navigator::Config navCfg{geometry().getTG()};
  navCfg.resolveMaterial = true;
  navCfg.resolvePassive = false;
  navCfg.resolveSensitive = true;
  const Acts::Navigator navigator(navCfg);

  auto gsf_propagator =
      GsfPropagator(std::move(multi_stepper), std::move(navigator),
                    Acts::getDefaultLogger("GSF_PROP", acts_loggingLevel));

  BetheHeitlerApprox betheHeitler = Acts::makeDefaultBetheHeitlerApprox();

  const auto gsfLogger = Acts::getDefaultLogger("GSF", acts_loggingLevel);

  gsf_ = std::make_unique<std::decay_t<decltype(*gsf_)>>(
      std::move(gsf_propagator), std::move(betheHeitler),
      Acts::getDefaultLogger("GSF", acts_loggingLevel));

  const auto stepper = Acts::EigenStepper<>{map};
  propagator_ = std::make_unique<Propagator>(
      stepper, navigator, Acts::getDefaultLogger("PROP", acts_loggingLevel));

  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(
      *propagator_, geometry_context(), magnetic_field_context());
}

void GSFProcessor::configure(framework::config::Parameters& parameters) {
  out_trk_collection_ =
      parameters.getParameter<std::string>("out_trk_collection", "GSFTracks");

  trackCollection_ =
      parameters.getParameter<std::string>("trackCollection", "TaggerTracks");
  measCollection_ = parameters.getParameter<std::string>("measCollection",
                                                         "DigiTaggerSimHits");

  maxComponents_ = parameters.getParameter<int>("maxComponents", 4);
  abortOnError_ = parameters.getParameter<bool>("abortOnError", false);
  disableAllMaterialHandling_ =
      parameters.getParameter<bool>("disableAllMaterialHandling", false);
  weightCutoff_ = parameters.getParameter<double>("weightCutoff_", 1.0e-4);

  propagator_maxSteps_ =
      parameters.getParameter<int>("propagator_maxSteps", 10000);
  propagator_step_size_ =
      parameters.getParameter<double>("propagator_step_size", 200.);
  field_map_ = parameters.getParameter<std::string>("field_map");
  usePerigee_ = parameters.getParameter<bool>("usePerigee", false);

  debug_ = parameters.getParameter<bool>("debug", false);
  taggerTracking_ = parameters.getParameter<bool>("taggerTracking", true);

  // finalReductionMethod_ =
  // parameters.getParameter<double>("finalReductionMethod",);
}

void GSFProcessor::produce(framework::Event& event) {
  // General Setup

  auto tg{geometry()};

  // Retrieve the tracks
  if (!event.exists(trackCollection_)) return;
  auto tracks{event.getCollection<ldmx::Track>(trackCollection_)};

  // Retrieve the measurements
  if (!event.exists(measCollection_)) return;
  auto measurements{event.getCollection<ldmx::Measurement>(measCollection_)};

  tracking::sim::LdmxMeasurementCalibrator calibrator{measurements};

  // GSF Setup
  Acts::GainMatrixUpdater updater;
  Acts::GsfExtensions<Acts::VectorMultiTrajectory> gsf_extensions;
  gsf_extensions.updater.connect<
      &Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(
      &updater);
  gsf_extensions.calibrator
      .connect<&tracking::sim::LdmxMeasurementCalibrator::calibrate_1d<
          Acts::VectorMultiTrajectory>>(&calibrator);

  // Surface Accessor
  struct SurfaceAccessor {
    const Acts::TrackingGeometry* trackingGeometry;

    const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const {
      const auto& indexSourceLink =
          sourceLink.get<ActsExamples::IndexSourceLink>();
      return trackingGeometry->findSurface(indexSourceLink.geometryId());
    }
  };

  SurfaceAccessor m_slSurfaceAccessor{tg.getTG().get()};
  // m_slSurfaceAccessor.trackingGeometry = tg.getTG();
  gsf_extensions.surfaceAccessor.connect<&SurfaceAccessor::operator()>(
      &m_slSurfaceAccessor);
  gsf_extensions.mixtureReducer.connect<&Acts::reduceMixtureLargestWeights>();

  // Propagator Options

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
  if (taggerTracking_) {
    reference_surface = tagger_layer_surface;
  }

  /*
  Acts::GsfOptions<Acts::VectorMultiTrajectory> gsfOptions{
      geometry_context(),    magnetic_field_context(),
      calibration_context(), gsf_extensions,
      propagator_options,    &(*origin_surface),
      maxComponents_,        weightCutoff_,
      abortOnError_,         disableAllMaterialHandling_};
  */
  Acts::GsfOptions<Acts::VectorMultiTrajectory> gsfOptions{
      geometry_context(), magnetic_field_context(), calibration_context()};

  gsfOptions.extensions = gsf_extensions;
  gsfOptions.propagatorPlainOptions = propagator_options;
  gsfOptions.referenceSurface = &(*reference_surface);
  gsfOptions.maxComponents = maxComponents_;
  gsfOptions.weightCutoff = weightCutoff_;
  gsfOptions.abortOnError = abortOnError_;
  gsfOptions.disableAllMaterialHandling = disableAllMaterialHandling_;

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
    std::vector<ldmx::Measurement> measOnTrack;

    // std::vector<ActsExamples::IndexSourceLink> fit_trackSourceLinks;
    std::vector<Acts::SourceLink> fit_trackSourceLinks;

    for (auto imeas : track.getMeasurementsIdxs()) {
      auto meas = measurements.at(imeas);
      measOnTrack.push_back(meas);

      // Retrieve the surface

      const Acts::Surface* hit_surface = tg.getSurface(meas.getLayerID());

      // Store the index source link
      ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
      fit_trackSourceLinks.push_back(Acts::SourceLink(idx_sl));
    }

    // Reverse the order of the vectors
    std::reverse(measOnTrack.begin(), measOnTrack.end());
    std::reverse(fit_trackSourceLinks.begin(), fit_trackSourceLinks.end());

    for (auto m : measOnTrack) {
      ldmx_log(debug) << "Measurement:\n" << m << "\n";
    }

    ldmx_log(debug) << "GSF Refitting";

    // Get the track parameters

    std::shared_ptr<Acts::PerigeeSurface> perigee =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
            track.getPerigeeX(), track.getPerigeeY(), track.getPerigeeZ()));

    Acts::BoundTrackParameters trk_btp =
        tracking::sim::utils::boundTrackParameters(track, perigee);

    std::shared_ptr<Acts::Surface> beamOrigin_surface =
        tracking::sim::utils::unboundSurface(-700);

    const std::shared_ptr<Acts::Surface> target_surface =
        tracking::sim::utils::unboundSurface(0.);

    const std::shared_ptr<Acts::Surface> ecal_surface =
        tracking::sim::utils::unboundSurface(240.5);

    Acts::BoundTrackParameters trk_btp_bO =
        tracking::sim::utils::boundTrackParameters(track, perigee);

    if (taggerTracking_) {
      if (!track.getTrackState(ldmx::TrackStateType::AtBeamOrigin)
               .has_value()) {
        ldmx_log(warn) << "Failed retreiving AtBeamOrigin TrackState for "
                          "track. Skipping..";
        continue;
      }

      auto ts = track.getTrackState(ldmx::TrackStateType::AtBeamOrigin).value();
      trk_btp_bO = tracking::sim::utils::btp(
          ts, beamOrigin_surface,
          11);  // 11 == electron PDGid...hardcode for now
    } else {
      if (!track.getTrackState(ldmx::TrackStateType::AtTarget).has_value()) {
        ldmx_log(warn)
            << "Failed retreiving AtTarget TrackState for track. Skipping..";
        continue;
      }
      auto ts = track.getTrackState(ldmx::TrackStateType::AtTarget).value();
      trk_btp_bO = tracking::sim::utils::btp(ts, target_surface, 11);
    }
    const Acts::BoundVector& trkpars = trk_btp.parameters();
    ldmx_log(debug) << "CKF Track parameters" << std::endl
                    << trkpars[0] << " " << trkpars[1] << " " << trkpars[2]
                    << " " << trkpars[3] << " " << trkpars[4] << " "
                    << trkpars[5] << std::endl
                    << "Perigee Surface" << std::endl
                    << track.getPerigeeX() << " " << track.getPerigeeY() << " "
                    << track.getPerigeeZ();

    Acts::Vector3 trk_pos = trk_btp.position(geometry_context());

    ldmx_log(debug) << trk_pos(0) << " " << trk_pos(1) << " " << trk_pos(2)
                    << std::endl;

    const Acts::BoundVector& trkpars_bO = trk_btp_bO.parameters();
    ldmx_log(debug) << "CKF BeamOrigin track parameters" << std::endl
                    << trkpars_bO[0] << " " << trkpars_bO[1] << " "
                    << trkpars_bO[2] << " " << trkpars_bO[3] << " "
                    << trkpars_bO[4] << " " << trkpars_bO[5] << " ";

    Acts::Vector3 trk_pos_bO = trk_btp_bO.position(geometry_context());
    ldmx_log(debug) << trk_pos_bO(0) << " " << trk_pos_bO(1) << " "
                    << trk_pos_bO(2) << std::endl;

    auto gsf_refit_result =
        gsf_->fit(fit_trackSourceLinks.begin(), fit_trackSourceLinks.end(),
                  trk_btp_bO, gsfOptions, tc);

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
        << " " << perigee_surface.transform(geometry_context()).translation()(0)
        << " " << perigee_surface.transform(geometry_context()).translation()(1)
        << " " << perigee_surface.transform(geometry_context()).translation()(2)
        << std::endl;

    ldmx::Track trk = ldmx::Track();

    bool success = false;
    if (taggerTracking_) {
      ldmx_log(debug) << "Target extrapolation";
      ldmx::Track::TrackState tsAtTarget;

      success = trk_extrap_->TrackStateAtSurface(
          gsftrk, target_surface, tsAtTarget, ldmx::TrackStateType::AtTarget);

      if (success) trk.addTrackState(tsAtTarget);
    } else {
      ldmx_log(debug) << "Ecal Extrapolation";
      ldmx::Track::TrackState tsAtEcal;
      success = trk_extrap_->TrackStateAtSurface(gsftrk, ecal_surface, tsAtEcal,
                                                 ldmx::TrackStateType::AtECAL);

      if (success) trk.addTrackState(tsAtEcal);
    }

    trk.setPerigeeLocation(
        perigee_surface.transform(geometry_context()).translation()(0),
        perigee_surface.transform(geometry_context()).translation()(1),
        perigee_surface.transform(geometry_context()).translation()(2));

    trk.setChi2(gsftrk.chi2());
    trk.setNhits(gsftrk.nMeasurements());
    trk.setNdf(gsftrk.nMeasurements() - 5);
    trk.setPerigeeParameters(
        tracking::sim::utils::convertActsToLdmxPars(perigee_pars));
    std::vector<double> v_trk_cov;
    tracking::sim::utils::flatCov(trk_cov, v_trk_cov);
    trk.setPerigeeCov(v_trk_cov);
    Acts::Vector3 trk_momentum = gsftrk.momentum();
    trk.setMomentum(trk_momentum(0), trk_momentum(1), trk_momentum(2));

    // truth information
    trk.setTrackID(track.getTrackID());
    trk.setPdgID(track.getPdgID());
    trk.setTruthProb(track.getTruthProb());

    itrk++;

    out_tracks.push_back(trk);

  }  // loop on tracks

  event.add(out_trk_collection_, out_tracks);
}

void GSFProcessor::onProcessStart(){};
void GSFProcessor::onProcessEnd(){};

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, GSFProcessor)
