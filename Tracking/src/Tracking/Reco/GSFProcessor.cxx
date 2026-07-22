#include "Tracking/Reco/GSFProcessor.h"

#include <algorithm>
#include <chrono>
#include <iomanip>

#include "Acts/EventData/SourceLink.hpp"
#include "Tracking/Event/Track.h"

namespace tracking {
namespace reco {

GSFProcessor::GSFProcessor(const std::string& name, framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void GSFProcessor::onNewRun(const ldmx::RunHeader& rh) {
  beam_origin_surface_ = tracking::sim::utils::unboundSurface(-700);
  // 1mm inside tagger ACTS volume outer boundary (~-618mm), 1.5mm upstream of L1 at x=-615.5mm
  tagger_start_surface_ = tracking::sim::utils::unboundSurface(-617.);
  target_surface_ = tracking::sim::utils::unboundSurface(0.);
  ecal_surface_ = tracking::sim::utils::unboundSurface(240.5);

  // Setup a interpolated bfield map
  if (field_map_.empty())
    loadBField();
  else
    loadBField(field_map_);
  const auto map =
      std::static_pointer_cast<InterpolatedMagneticField3>(bField());

  auto acts_logging_level = Acts::Logging::FATAL;

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

  auto bethe_heitler = std::make_shared<Acts::PolynomialBetheHeitlerApprox>(
      Acts::makeDefaultBetheHeitlerApprox());

  gsf_ = std::make_unique<std::decay_t<decltype(*gsf_)>>(
      std::move(gsf_propagator), std::move(bethe_heitler),
      Acts::getDefaultLogger("GSF", acts_logging_level));

  const auto stepper = Acts::EigenStepper<>{map};
  propagator_ = std::make_unique<Propagator>(
      stepper, navigator,
      Acts::getDefaultLogger("GSF_EXTRAP", acts_logging_level));

  propagator_extrap_ = std::make_unique<GsfExtrapPropagator>(
      Acts::EigenStepper<>{map}, Acts::VoidNavigator{});
  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(
      *propagator_extrap_, geometryContext(), magneticFieldContext());
}

void GSFProcessor::configure(framework::config::Parameters& parameters) {
  out_trk_collection_ =
      parameters.get<std::string>("out_trk_collection", "GSFTracks");

  track_collection_ =
      parameters.get<std::string>("track_collection", "TaggerTracks");
  meas_collection_ =
      parameters.get<std::string>("meas_collection", "DigiTaggerSimHits");

  track_passname_ = parameters.get<std::string>("track_passname");
  meas_passname_ = parameters.get<std::string>("meas_passname");
  track_collection_event_passname_ =
      parameters.get<std::string>("track_collection_event_passname");
  meas_collection_event_passname_ =
      parameters.get<std::string>("meas_collection_event_passname");

  max_components_ = parameters.get<int>("max_components", 4);
  abort_on_error_ = parameters.get<bool>("abort_on_error", false);
  disable_all_material_handling_ =
      parameters.get<bool>("disable_all_material_handling", false);
  weight_cutoff_ = parameters.get<double>("weight_cutoff_", 1.0e-4);

  propagator_max_steps_ = parameters.get<int>("propagator_max_steps", 10000);
  propagator_step_size_ = parameters.get<double>("propagator_step_size", 200.);
  field_map_ = parameters.get<std::string>("field_map");
  use_perigee_ = parameters.get<bool>("usePerigee", false);

  debug_ = parameters.get<bool>("debug", false);
  tagger_tracking_ = parameters.get<bool>("tagger_tracking", true);

  // final_reduction_method_ =
  // parameters.get<double>("finalReductionMethod",);
}  // end of configure()

void GSFProcessor::produce(framework::Event& event) {
  auto t_start = std::chrono::high_resolution_clock::now();

  // General Setup

  auto tg{geometry()};

  // Retrieve the tracks
  if (!event.exists(track_collection_, track_collection_event_passname_))
    return;
  const auto& tracks =
      event.getCollection<ldmx::Track>(track_collection_, track_passname_);

  // Retrieve the measurements
  if (!event.exists(meas_collection_, meas_collection_event_passname_)) return;
  const auto& measurements =
      event.getCollection<ldmx::Measurement>(meas_collection_, meas_passname_);

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

  // Electron hypothesis
  //  propagator_options.mass = 0.511 * Acts::UnitConstants::MeV;

  // GSF options will be configured per-track
  std::shared_ptr<const Acts::Surface> gsf_ref_surface;
  Acts::GsfOptions<Acts::VectorMultiTrajectory> gsf_options{
      geometryContext(), magneticFieldContext(), calibrationContext()};
  gsf_options.extensions = gsf_extensions;
  gsf_options.propagatorPlainOptions =
      static_cast<Acts::PropagatorPlainOptions>(propagator_options);
  gsf_options.maxComponents = max_components_;
  gsf_options.weightCutoff = weight_cutoff_;
  gsf_options.abortOnError = abort_on_error_;
  gsf_options.disableAllMaterialHandling = disable_all_material_handling_;

  // Output track container
  std::vector<ldmx::Track> out_tracks;

  Acts::VectorTrackContainer vtc;
  Acts::VectorMultiTrajectory mtj;
  Acts::TrackContainer tc{vtc, mtj};

  // Loop on tracks
  n_input_tracks_ += static_cast<int>(tracks.size());

  unsigned int itrk = 0;
  int n_gsf_ok_evt = 0;
  int n_tgt_fail_evt = 0;
  ldmx_log(debug) << "Starting GSF processing of " << tracks.size()
                  << " tracks";

  for (auto& track : tracks) {
    ldmx_log(debug) << "Processing track " << itrk << " with "
                    << track.getMeasurementsIdxs().size() << " measurements";
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

      // Store the index source link
      acts_examples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
      fit_track_source_links.push_back(Acts::SourceLink(idx_sl));
    }

    // Reverse the order of the vectors
    std::reverse(meas_on_track.begin(), meas_on_track.end());
    std::reverse(fit_track_source_links.begin(), fit_track_source_links.end());

    for (auto m : meas_on_track) {
      ldmx_log(trace) << "    Measurement:\n" << m << "\n";
    }

    ldmx_log(debug) << "  Track bound track parameters preparation:";

    // Reconstruct BoundTrackParameters at perigee (target) from stored params.
    // perigee_ is stored in LDMX frame; rotate to ACTS frame for surface
    // creation.
    Acts::Vector3 perigee_acts = tracking::sim::utils::ldmx2Acts(Acts::Vector3(
        track.getPerigeeX(), track.getPerigeeY(), track.getPerigeeZ()));
    std::shared_ptr<Acts::PerigeeSurface> perigee =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(perigee_acts);

    Acts::BoundTrackParameters trk_btp =
        tracking::sim::utils::boundTrackParameters(track, perigee);

    Acts::BoundTrackParameters trk_btp_fit_start = trk_btp;


    // For tagger: backward-extrapolate (via VoidNavigator) from the target perigee
    // (x=0mm) to just inside the tagger outer boundary (x≈-650mm), then run the
    // GSF forward (+x) through L1→L7.  The CKF stores tagger track perigees at
    // the target, so we must back-propagate before handing off to the GSF.
    if (tagger_tracking_) {
      auto opt_tagger_start =
          trk_extrap_->extrapolate(trk_btp, tagger_start_surface_);
      if (!opt_tagger_start) {
        ldmx_log(debug) << "  Failed pre-fit extrapolation to tagger start surface (itrk=" << itrk << ")";
        ++n_gsf_failed_;
        continue;
      }
      trk_btp_fit_start = *opt_tagger_start;
    }

    ldmx_log(debug) << "    Perigee surface (acts): (" << track.getPerigeeX()
                    << ", " << track.getPerigeeY() << ", "
                    << track.getPerigeeZ() << ")";

    const Acts::BoundVector& trkpars = trk_btp.parameters();
    ldmx_log(debug) << "    Perigee parameters (d0, z0, phi, theta, q/p)= ("
                    << trkpars[Acts::eBoundLoc0] << ", "
                    << trkpars[Acts::eBoundLoc1] << ", "
                    << trkpars[Acts::eBoundPhi] << ", "
                    << trkpars[Acts::eBoundTheta] << ", "
                    << trkpars[Acts::eBoundQOverP] << ")";

    const Acts::BoundVector& fit_start_pars = trk_btp_fit_start.parameters();
    ldmx_log(debug) << "    GSF start parameters (d0, z0, phi, theta, q/p)= ("
                    << fit_start_pars[Acts::eBoundLoc0] << ", "
                    << fit_start_pars[Acts::eBoundLoc1] << ", "
                    << fit_start_pars[Acts::eBoundPhi] << ", "
                    << fit_start_pars[Acts::eBoundTheta] << ", "
                    << fit_start_pars[Acts::eBoundQOverP] << ")";

    ldmx_log(debug) << "  About to run GSF fit with "
                    << fit_track_source_links.size() << " source links";

    // GSF reference surface: for tagger use the start surface (x=-648mm, inside
    // geometry), for recoil use the target (x=0mm).
    if (tagger_tracking_) {
      gsf_ref_surface = tagger_start_surface_;
    } else {
      gsf_ref_surface = target_surface_;
    }
    gsf_options.referenceSurface = &(*gsf_ref_surface);

    auto gsf_refit_result =
        gsf_->fit(fit_track_source_links.begin(), fit_track_source_links.end(),
                  trk_btp_fit_start, gsf_options, tc);

    if (!gsf_refit_result.ok()) {
      ldmx_log(debug) << "  GSF re-fit failed (itrk=" << itrk
                      << "): " << gsf_refit_result.error().message();
      if (n_gsf_failed_ < 5)
        ldmx_log(info) << "  [GSF dbg] fit failed (first few): "
                       << gsf_refit_result.error().message();
      ++n_gsf_failed_;
      continue;
    }

    ++n_gsf_ok_evt;
    ldmx_log(debug) << "  GSF fit succeeded (itrk=" << itrk
                    << "), tc.size()=" << tc.size();

    auto gsftrk = gsf_refit_result.value();
    // calculateTrackQuantities(gsftrk);

    const Acts::BoundVector& perigee_pars = gsftrk.parameters();
    const Acts::BoundMatrix& trk_cov = gsftrk.covariance();
    const Acts::Surface& perigee_surface = gsftrk.referenceSurface();

    ldmx_log(debug)
        << "    Reference Surface (acts-x, acts-y, acts-z) = ("
        << perigee_surface.localToGlobalTransform(geometryContext()).translation()(0) << ", "
        << perigee_surface.localToGlobalTransform(geometryContext()).translation()(1) << ", "
        << perigee_surface.localToGlobalTransform(geometryContext()).translation()(2) << ")";

    ldmx_log(debug) << "    nTrackStates=" << gsftrk.nTrackStates()
                    << " nMeasurements=" << gsftrk.nMeasurements()
                    << " chi2=" << gsftrk.chi2();
    if (gsftrk.nTrackStates() == 0)
      ldmx_log(info) << "  [GSF dbg] track has 0 states after fit (itrk="
                     << itrk << ");";

    ldmx_log(debug) << "    Track parameters (d0, z0, phi, theta, q/p)= ("
                    << perigee_pars[Acts::eBoundLoc0] << ", "
                    << perigee_pars[Acts::eBoundLoc1] << ", "
                    << perigee_pars[Acts::eBoundPhi] << ", "
                    << perigee_pars[Acts::eBoundTheta] << ", "
                    << perigee_pars[Acts::eBoundQOverP] << ") ";

    ldmx::Track trk;

    // Extrapolate GSF track to target surface to get perigee parameters
    auto opt_target = trk_extrap_->extrapolate(gsftrk, target_surface_);

    ldmx_log(debug) << "    Extrapolating to target (itrk=" << itrk << ")";
    if (opt_target) {
      ldmx_log(debug) << "    GSF target extrapolation succeeded";
      auto ts_at_target = tracking::sim::utils::makeTrackState(
          geometryContext(), *opt_target, ldmx::AtTarget);
      trk.addTrackState(ts_at_target);

      trk.setPerigeeParameters(tracking::sim::utils::convertActsToLdmxPars(
          opt_target->parameters()));
      if (opt_target->covariance()) {
        std::vector<double> cov_vec;
        tracking::sim::utils::flatCov(*(opt_target->covariance()), cov_vec);
        trk.setPerigeeCov(cov_vec);
      }
      Acts::Vector3 target_loc_ldmx = tracking::sim::utils::acts2Ldmx(
          target_surface_->localToGlobalTransform(geometryContext()).translation());
      trk.setPerigeeLocation(target_loc_ldmx[0], target_loc_ldmx[1],
                             target_loc_ldmx[2]);

      ldmx_log(debug)
          << "    GSF target parameters (d0, z0, phi, theta, q/p)= ("
          << opt_target->parameters()[Acts::eBoundLoc0] << ", "
          << opt_target->parameters()[Acts::eBoundLoc1] << ", "
          << opt_target->parameters()[Acts::eBoundPhi] << ", "
          << opt_target->parameters()[Acts::eBoundTheta] << ", "
          << opt_target->parameters()[Acts::eBoundQOverP] << ")";
    } else {
      ++n_target_extrap_failed_;
      ++n_tgt_fail_evt;
      ldmx_log(debug) << "    GSF target extrapolation failed (itrk=" << itrk
                      << "), using GSF fit parameters at reference surface";
      trk.setPerigeeParameters(
          tracking::sim::utils::convertActsToLdmxPars(perigee_pars));
      std::vector<double> v_trk_cov;
      tracking::sim::utils::flatCov(trk_cov, v_trk_cov);
      trk.setPerigeeCov(v_trk_cov);
    }

    // Tagger: also add beam-origin state; Recoil: add ECAL state
    if (tagger_tracking_) {
      auto opt_beam_origin =
          trk_extrap_->extrapolate(gsftrk, beam_origin_surface_);
      if (opt_beam_origin)
        trk.addTrackState(tracking::sim::utils::makeTrackState(
            geometryContext(), *opt_beam_origin, ldmx::AtBeamOrigin));
    } else {
      ldmx_log(debug) << "  ECAL extrapolation";
      auto opt_ecal = trk_extrap_->extrapolate(gsftrk, ecal_surface_);
      if (opt_ecal)
        trk.addTrackState(tracking::sim::utils::makeTrackState(
            geometryContext(), *opt_ecal, ldmx::AtECAL));
      else
        ++n_ecal_extrap_failed_;
    }

    trk.setChi2(gsftrk.chi2());
    trk.setNhits(gsftrk.nMeasurements());
    trk.setNdf(gsftrk.nMeasurements() - 5);
    trk.setCharge(perigee_pars[Acts::eBoundQOverP] > 0 ? 1 : -1);

    // Truth information carried over from input track
    trk.setTrackID(track.getTrackID());
    trk.setPdgID(track.getPdgID());
    trk.setTruthProb(track.getTruthProb());

    itrk++;

    ldmx_log(debug) << "  Added track to output, total tracks = "
                    << (out_tracks.size() + 1);

    out_tracks.push_back(trk);

  }  // loop on tracks

  ldmx_log(debug) << "[GSF evt " << nevents_ << "] in=" << tracks.size()
                  << " gsf_ok=" << n_gsf_ok_evt
                  << " tgt_fail=" << n_tgt_fail_evt
                  << " out=" << out_tracks.size();

  n_output_tracks_ += static_cast<int>(out_tracks.size());
  event.add(out_trk_collection_, out_tracks);

  auto t_end = std::chrono::high_resolution_clock::now();
  processing_time_ +=
      std::chrono::duration<double, std::milli>(t_end - t_start).count();
  ++nevents_;
}  // end of produce()

void GSFProcessor::onProcessStart() {}

void GSFProcessor::onProcessEnd() {
  ldmx_log(info) << "--------------------------------- ";
  ldmx_log(info) << "GSF: " << n_output_tracks_ << " output tracks / "
                 << n_input_tracks_ << " input tracks";
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / nevents_ << " ms";
  ldmx_log(info) << "GSF Fit Failures: " << n_gsf_failed_;
  ldmx_log(info) << "Extrapolation Failures::";
  ldmx_log(info) << "  Target: " << n_target_extrap_failed_ << " times";
  if (!tagger_tracking_)
    ldmx_log(info) << "  ECAL:   " << n_ecal_extrap_failed_ << " times";
}

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::GSFProcessor)
