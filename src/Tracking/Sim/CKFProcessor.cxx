#include "Tracking/Sim/CKFProcessor.h"

#include "SimCore/Event/SimParticle.h"
#include "Tracking/Sim/GeometryContainers.h"

//--- C++ StdLib ---//
#include <algorithm>  //std::vector reverse
#include <iostream>

// eN files
#include <fstream>

namespace tracking {
namespace sim {

CKFProcessor::CKFProcessor(const std::string& name, framework::Process& process)
    : framework::Producer(name, process) {
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);
}

CKFProcessor::~CKFProcessor() {}

void CKFProcessor::onProcessStart() {
  profiling_map_["setup"] = 0.;
  profiling_map_["hits"] = 0.;
  profiling_map_["seeds"] = 0.;
  profiling_map_["ckf_setup"] = 0.;
  profiling_map_["ckf_run"] = 0.;
  profiling_map_["result_loop"] = 0.;

  gctx_ = Acts::GeometryContext();
  bctx_ = Acts::MagneticFieldContext();

  // Build the tracking geometry
  ldmx_tg = std::make_shared<tracking::reco::TrackersTrackingGeometry>(
      "/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12/detector.gdml",
      &gctx_, debug_);
  const auto tGeometry = ldmx_tg->getTG();

  if (dumpobj_) ldmx_tg->dumpGeometry("./");

  // Seed the generator
  generator_.seed(1);

  //==> Move to a separate processor <== //

  // Generate a constant magnetic field
  Acts::Vector3 b_field(0., 0., bfield_ * Acts::UnitConstants::T);

  // Setup a constant magnetic field
  const auto constBField = std::make_shared<Acts::ConstantBField>(b_field);

  // TODO:: Move this to an external file
  auto localToGlobalBin_xyz = [](std::array<size_t, 3> bins,
                                 std::array<size_t, 3> sizes) {
    return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] +
            bins[2]);  // xyz - field space
    // return (bins[1] * (sizes[2] * sizes[0]) + bins[2] * sizes[0] + bins[0]);
    // //zxy
  };

  // Setup a interpolated bfield map
  const auto map = std::make_shared<InterpolatedMagneticField3>(
      makeMagneticFieldMapXyzFromText(
          std::move(localToGlobalBin_xyz), bfieldMap_,
          1. * Acts::UnitConstants::mm,    // default scale for axes length
          1000. * Acts::UnitConstants::T,  // The map is in kT, so scale it to T
          false,                           // not symmetrical
          true                             // rotate the axes to tracking frame
          ));

  // Setup the steppers
  const auto stepper = Acts::EigenStepper<>{map};
  const auto const_stepper = Acts::EigenStepper<>{constBField};
  const auto multi_stepper = Acts::MultiEigenStepperLoop{map};

  // Setup the navigator
  Acts::Navigator::Config navCfg{tGeometry};
  navCfg.resolveMaterial = true;
  navCfg.resolvePassive = true;
  navCfg.resolveSensitive = true;
  const Acts::Navigator navigator(navCfg);

  // Setup the propagators
  propagator_ = const_b_field_
                    ? std::make_unique<CkfPropagator>(const_stepper, navigator)
                    : std::make_unique<CkfPropagator>(stepper, navigator);
  auto gsf_propagator = GsfPropagator(multi_stepper, navigator);

  // Setup the fitters
  ckf_ = std::make_unique<std::decay_t<decltype(*ckf_)>>(*propagator_);
  kf_ = std::make_unique<std::decay_t<decltype(*kf_)>>(*propagator_);
  gsf_ = std::make_unique<std::decay_t<decltype(*gsf_)>>(
      std::move(gsf_propagator));

  // Setup the propagator steps writer
  PropagatorStepWriter::Config cfg;
  cfg.filePath = steps_outfile_path_;

  writer_ = std::make_unique<PropagatorStepWriter>(cfg);

  // Create a mapping between the layers and the Acts::Surface
  // layer_surface_map_ = makeLayerSurfacesMap(tGeometry);

  // Prepare histograms

  // getHistoDirectory();

  // Track parameters
  // histograms_.create("d0_reco","d0_reco",100,-20,20);
  // histograms_.create("z0_reco","z0_reco",100,-20,20);
  // histograms_.create("q/p","q_over_p",100,-1,1);
  // histograms_.create("p","p",100,0,10);
  // histograms_.create("theta","theta",100,-3.15,3.15);
  // histograms_.create("phi","phi",100,-3.15,3.15);
  // histograms_.create("t","t",100,0,100);

  // Residuals
  // histograms_.create("d0_res","d0_reco",100,-5,5);
  // histograms_.create("z0_res","z0_reco",100,-5,5);
  // histograms_.create("q/p_res","q_over_p",100,-0.1,0.1);
  // histograms_.create("p_res","p",100,-2,2);
  // histograms_.create("theta_res","theta",100,-0.1,0.1);
  // histograms_.create("phi_res","phi",100,-0.1,0.1);
  // histograms_.create("t_res","t",100,-0.1,0.1);

  // Validation histograms
  double ptlim = 0.1;
  histo_p_ = std::make_unique<TH1F>("p_res", "p_res", 200, -1, 1);

  histo_px_ = std::make_unique<TH1F>("px_res", "px_res", 200, -1, 1);
  histo_py_ = std::make_unique<TH1F>("py_res", "py_res", 200, -ptlim, ptlim);
  histo_pz_ = std::make_unique<TH1F>("pz_res", "pz_res", 200, -ptlim, ptlim);
  histo_pt_ = std::make_unique<TH1F>("pt_res", "pt_res", 200, -ptlim, ptlim);

  histo_d0_ = std::make_unique<TH1F>("d0_res", "d0_res", 200, -0.2, 0.2);
  histo_z0_ = std::make_unique<TH1F>("z0_res", "z0_res", 200, -0.75, 0.75);
  histo_phi_ = std::make_unique<TH1F>("phi_res", "phi_res", 200, -0.015, 0.015);
  histo_theta_ =
      std::make_unique<TH1F>("theta_res", "theta_res", 200, -0.01, 0.01);
  histo_qop_ = std::make_unique<TH1F>("qop_res", "qop_res", 200, -5, 5);

  histo_py_vs_p_ = std::make_unique<TH2F>("py_vs_p", "py_vs_p", 200, 0, 6, 200,
                                          -ptlim, ptlim);
  histo_py_vs_py_ = std::make_unique<TH2F>("py_vs_py", "py_vs_py", 200, 0, 2,
                                           200, -ptlim, ptlim);
  histo_pz_vs_p_ = std::make_unique<TH2F>("pz_vs_p", "pz_vs_p", 200, 0, 6, 200,
                                          -ptlim, ptlim);
  histo_pz_vs_pz_ = std::make_unique<TH2F>("pz_vs_pz", "pz_vs_pz", 200, 0, 2,
                                           200, -ptlim, ptlim);
  histo_pt_vs_p_ = std::make_unique<TH2F>("pt_vs_p", "pt_vs_p", 200, 0, 2, 200,
                                          -ptlim, ptlim);
  histo_pt_vs_pt_ = std::make_unique<TH2F>("pt_vs_pt", "pt_vs_pt", 200, 0, 2,
                                           200, -ptlim, ptlim);

  histo_p_pull_ = std::make_unique<TH1F>("p_pull", "p_pull", 200, -5, 5);
  histo_d0_pull_ = std::make_unique<TH1F>("d0_pull", "d0_pull", 200, -5, 5);
  histo_z0_pull_ = std::make_unique<TH1F>("z0_pull", "z0_pull", 200, -5, 5);
  histo_phi_pull_ = std::make_unique<TH1F>("phi_pull", "phi_pull", 200, -5, 5);
  histo_theta_pull_ =
      std::make_unique<TH1F>("theta_pull", "theta_pull", 200, -5, 5);
  histo_qop_pull_ = std::make_unique<TH1F>("qop_pull", "qop_pull", 200, -5, 5);

  h_p_ = std::make_unique<TH1F>("p", "p", 600, 0, 6);
  h_px_ = std::make_unique<TH1F>("px", "px", 600, 0, 6);
  h_py_ = std::make_unique<TH1F>("py", "py", 200, -ptlim, ptlim);
  h_pz_ = std::make_unique<TH1F>("pz", "pz", 200, -ptlim, ptlim);
  h_pt_ = std::make_unique<TH1F>("pt", "pt", 200, 0, ptlim);

  h_d0_ = std::make_unique<TH1F>("d0", "d0", 100, -20, 20);
  h_z0_ = std::make_unique<TH1F>("z0", "z0", 100, -50, 50);
  h_phi_ = std::make_unique<TH1F>("phi", "phi", 200, -0.5, 0.5);
  h_theta_ = std::make_unique<TH1F>("theta", "theta", 200, 0.8, 2.2);
  h_qop_ = std::make_unique<TH1F>("qop", "qop", 200, -10, 10);
  h_nHits_ = std::make_unique<TH1F>("nHits", "nHits", 15, 0, 15);

  h_p_err_ = std::make_unique<TH1F>("p_err", "p_err", 600, 0, 1);
  h_d0_err_ = std::make_unique<TH1F>("d0_err", "d0_err", 100, 0, 0.05);
  h_z0_err_ = std::make_unique<TH1F>("z0_err", "z0_err", 100, 0, 0.8);
  h_phi_err_ = std::make_unique<TH1F>("phi_err", "phi_err", 200, 0, 0.002);
  h_theta_err_ = std::make_unique<TH1F>("theta_err", "theta_err", 200, 0, 0.02);
  h_qop_err_ = std::make_unique<TH1F>("qop_err", "qop_err", 200, 0, 0.1);

  h_p_refit_ = std::make_unique<TH1F>("p_refit", "p_refit", 600, 0, 6);
  h_d0_refit_ = std::make_unique<TH1F>("d0_refit", "d0_refit", 100, -20, 20);
  h_z0_refit_ = std::make_unique<TH1F>("z0_refit", "z0_refit", 100, -50, 50);
  h_phi_refit_ =
      std::make_unique<TH1F>("phi_refit", "phi_refit", 200, -0.5, 0.5);
  h_theta_refit_ =
      std::make_unique<TH1F>("theta_refit", "theta_refit", 200, 0.8, 2.2);

  h_p_gsf_refit_ =
      std::make_unique<TH1F>("p_gsf_refit", "p_gsf_refit", 600, 0, 6);
  h_d0_gsf_refit_ =
      std::make_unique<TH1F>("d0_gsf_refit", "d0_gsf_refit", 100, -20, 20);
  h_z0_gsf_refit_ =
      std::make_unique<TH1F>("z0_gsf_refit", "z0_gsf_refit", 100, -50, 50);
  h_phi_gsf_refit_ =
      std::make_unique<TH1F>("phi_gsf_refit", "phi_gsf_refit", 200, -0.5, 0.5);
  h_theta_gsf_refit_ = std::make_unique<TH1F>("theta_gsf_refit",
                                              "theta_gsf_refit", 200, 0.8, 2.2);

  h_p_gsf_refit_res_ =
      std::make_unique<TH1F>("p_gsf_res", "p_gsf_res", 200, -1, 1);
  h_qop_gsf_refit_res_ =
      std::make_unique<TH1F>("qop_gsf_res", "qop_gsf_res", 200, -5, 5);

  h_p_truth_ = std::make_unique<TH1F>("p_truth", "p_truth", 600, 0, 6);
  h_pt_truth_ = std::make_unique<TH1F>("pt_truth", "pt_truth", 200, 0., ptlim);
  h_px_truth_ = std::make_unique<TH1F>("px_truth", "px_truth", 600, -6, 6);
  h_py_truth_ =
      std::make_unique<TH1F>("py_truth", "py_truth", 200, -ptlim, ptlim);
  h_pz_truth_ =
      std::make_unique<TH1F>("pz_truth", "pz_truth", 200, -ptlim, ptlim);

  h_d0_truth_ = std::make_unique<TH1F>("d0_truth", "d0_truth", 100, -20, 20);
  h_z0_truth_ = std::make_unique<TH1F>("z0_truth_", "z0_truth", 100, -50, 50);
  h_phi_truth_ =
      std::make_unique<TH1F>("phi_truth", "phi_truth", 200, -0.5, 0.5);
  h_theta_truth_ =
      std::make_unique<TH1F>("theta_truth`", "theta_truth", 200, 0.8, 2.2);
  h_qop_truth_ = std::make_unique<TH1F>("qop_truth", "qop_truth", 200, -10, 10);

  h_tgt_scoring_x_y_ = std::make_unique<TH2F>(
      "tgt_scoring_x_y", "tgt_scoring_x_y", 100, -40, 40, 100, -40, 40);
  h_tgt_scoring_z_ =
      std::make_unique<TH1F>("tgt_scoring_z", "tgt_scoring_z", 100, 0, 10);
}

void CKFProcessor::produce(framework::Event& event) {
  // int counter=0
  // if (counter==0) {
  // propagateENstates(event,
  //                   "hadron_vars_piminus_1e6.txt",
  //                   "hadron_vars_piminus_1e6_out.root");
  // counter+=1;
  // }

  // TODO use global variable instead and call clear;
  std::vector<ldmx::Track> tracks;

  auto start = std::chrono::high_resolution_clock::now();

  nevents_++;
  if (nevents_ % 1000 == 0)
    ldmx_log(info) << "events processed:" << nevents_;

  /*
  std::shared_ptr<const Acts::PerigeeSurface> perigee_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(perigee_location_.at(0), perigee_location_.at(1),
                        perigee_location_.at(2)));
  */
                        
  auto loggingLevel = Acts::Logging::DEBUG;
  ACTS_LOCAL_LOGGER(
      Acts::getDefaultLogger("LDMX Tracking Goemetry Maker", loggingLevel));

  Acts::PropagatorOptions<ActionList, AbortList> propagator_options(
      gctx_, bctx_, Acts::LoggerWrapper{logger()});

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
  propagator_options.maxStepSize =
      propagator_step_size_ * Acts::UnitConstants::mm;
  propagator_options.maxSteps = propagator_maxSteps_;

  // Electron hypothesis
  propagator_options.mass = 0.511 * Acts::UnitConstants::MeV;

  // std::cout<<"Setting up the Kalman Filter Algorithm"<<std::endl;

  // #######################//
  // Kalman Filter algorithm//
  // #######################//

  // Step 1 - Form the source links

  // std::vector<ActsExamples::IndexSourceLink> sourceLinks;
  // a) Loop over the sim Hits

  auto setup = std::chrono::high_resolution_clock::now();
  profiling_map_["setup"] +=
      std::chrono::duration<double, std::milli>(setup - start).count();

  const std::vector<ldmx::SimTrackerHit> sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(hit_collection_);
  const auto ldmxsps = makeLdmxSpacepoints(sim_hits);

  // The mapping between the geometry identifier
  // and the IndexSourceLink that points to the hit
  const auto geoId_sl_map = makeGeoIdSourceLinkMap(ldmxsps);

  auto hits = std::chrono::high_resolution_clock::now();
  profiling_map_["hits"] +=
      std::chrono::duration<double, std::milli>(hits - setup).count();

  // ============   Setup the CKF  ============

  // Retrieve the seeds

  ldmx_log(debug) << "Retrieve the seeds::" << seed_coll_name_;
  

  const std::vector<ldmx::Track> seed_tracks =
      event.getCollection<ldmx::Track>(seed_coll_name_);

  // Run the CKF on each seed and produce a track candidate
  std::vector<Acts::BoundTrackParameters> startParameters;
  for (auto& seed : seed_tracks) {

    // Transform the seed track to bound parameters
    std::shared_ptr<Acts::PerigeeSurface> perigeeSurface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
            seed.getPerigeeX(), seed.getPerigeeY(), seed.getPerigeeZ()));

    Acts::BoundVector paramVec;
    paramVec << seed.getD0(), seed.getZ0(), seed.getPhi(), seed.getTheta(),
        seed.getQoP(), seed.getT();

    Acts::BoundSymMatrix covMat =
        tracking::sim::utils::unpackCov(seed.getPerigeeCov());

    ldmx_log(debug) 
        << "perigee" << std::endl
        << seed.getPerigeeX() << " " << seed.getPerigeeY() << " "
        << seed.getPerigeeZ() << std::endl
        << "start Parameters" << std::endl
        << paramVec;
    
    Acts::ActsScalar q = seed.getQoP() < 0 ? -1 * Acts::UnitConstants::e
                                           : Acts::UnitConstants::e;

    startParameters.push_back(
        Acts::BoundTrackParameters(perigeeSurface, paramVec, q, covMat));
  }

  if (startParameters.size() != 1) {
    std::vector<ldmx::Track> empty;
    event.add(out_trk_collection_, empty);
    return;
  }

  nseeds_++;

  for (auto& startParameter : startParameters) {
    // Already fill the plots with the truth information
    h_p_truth_->Fill(startParameters.at(0).absoluteMomentum());
    h_px_truth_->Fill(startParameters.at(0).momentum()(0));
    h_py_truth_->Fill(startParameters.at(0).momentum()(1));
    h_pz_truth_->Fill(startParameters.at(0).momentum()(2));

    double py_truth = startParameters.at(0).momentum()(1);
    double pz_truth = startParameters.at(0).momentum()(2);
    double pt_truth = sqrt(py_truth * py_truth + pz_truth * pz_truth);

    h_pt_truth_->Fill(pt_truth);

    h_d0_truth_->Fill(
        startParameters.at(0).get<Acts::BoundIndices::eBoundLoc0>());
    h_z0_truth_->Fill(
        startParameters.at(0).get<Acts::BoundIndices::eBoundLoc1>());
    h_phi_truth_->Fill(
        startParameters.at(0).get<Acts::BoundIndices::eBoundPhi>());
    h_theta_truth_->Fill(
        startParameters.at(0).get<Acts::BoundIndices::eBoundTheta>());
    h_qop_truth_->Fill(
        startParameters.at(0).get<Acts::BoundIndices::eBoundQOverP>());
  }

  auto seeds = std::chrono::high_resolution_clock::now();
  profiling_map_["seeds"] +=
      std::chrono::duration<double, std::milli>(seeds - hits).count();

  Acts::GainMatrixUpdater kfUpdater;
  Acts::GainMatrixSmoother kfSmoother;

  // configuration for the measurement selector. Empty geometry identifier means
  // applicable to all the detector elements

  Acts::MeasurementSelector::Config measurementSelectorCfg = {
      // global default: no chi2 cut, only one measurement per surface
      {Acts::GeometryIdentifier(),
       {{}, {std::numeric_limits<double>::max()}, {1u}}},
  };

  Acts::MeasurementSelector measSel{measurementSelectorCfg};
  LdmxMeasurementCalibrator calibrator{ldmxsps};

  Acts::CombinatorialKalmanFilterExtensions ckf_extensions;

  if (use1Dmeasurements_)
    ckf_extensions.calibrator.connect<&LdmxMeasurementCalibrator::calibrate_1d>(
        &calibrator);
  else
    ckf_extensions.calibrator.connect<&LdmxMeasurementCalibrator::calibrate>(
        &calibrator);
  ckf_extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(
      &kfUpdater);
  ckf_extensions.smoother.connect<&Acts::GainMatrixSmoother::operator()>(
      &kfSmoother);
  ckf_extensions.measurementSelector
      .connect<&Acts::MeasurementSelector::select>(&measSel);

  // Create source link accessor and connect delegate
  struct SourceLinkAccIt {
    using BaseIt = decltype(geoId_sl_map.begin());
    BaseIt it;

    using difference_type = typename BaseIt::difference_type;
    using iterator_category = typename BaseIt::iterator_category;
    using value_type = typename BaseIt::value_type::second_type;
    using pointer = typename BaseIt::pointer;
    using reference = value_type&;

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
    const value_type& operator*() const { return it->second; }
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

  std::shared_ptr<const Acts::PerigeeSurface> origin_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(0., 0., 0.));

  auto extr_surface = &(*origin_surface);

  std::shared_ptr<const Acts::PerigeeSurface> tgt_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(extrapolate_location_[0], extrapolate_location_[1],
                        extrapolate_location_[2]));

  if (use_extrapolate_location_) {
    extr_surface = &(*tgt_surface);
  }

  Acts::Vector3 seed_perigee_surface_center =
      startParameters.at(0).referenceSurface().center(gctx_);
  std::shared_ptr<const Acts::PerigeeSurface> seed_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          seed_perigee_surface_center);

  if (use_seed_perigee_) {
    extr_surface = &(*seed_surface);
  }

  auto ckf_loggingLevel = Acts::Logging::FATAL;
  if (debug_) ckf_loggingLevel = Acts::Logging::VERBOSE;
  const auto ckflogger = Acts::getDefaultLogger("CKF", ckf_loggingLevel);
  const Acts::CombinatorialKalmanFilterOptions<SourceLinkAccIt> ckfOptions(
      gctx_, bctx_, cctx_, sourceLinkAccessorDelegate, ckf_extensions,
      Acts::LoggerWrapper{*ckflogger}, propagator_options, &(*extr_surface));

  // run the CKF for all initial track states
  auto ckf_setup = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_setup"] +=
      std::chrono::duration<double, std::milli>(ckf_setup - seeds).count();

  auto results = ckf_->findTracks(startParameters, ckfOptions);

  auto ckf_run = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_run"] +=
      std::chrono::duration<double, std::milli>(ckf_run - ckf_setup).count();

  // std::cout<<"StartParameters size::"<<startParameters.size()<<std::endl;
  // std::cout<<"results size::"<<results.size()<<std::endl;

  int ResultIndex = 0;
  int GoodResult = 0;

  for (auto& result : results) {
    ResultIndex++;
    if (!result.ok()) {
      continue;
    }
    GoodResult++;

    auto ckf_result = result.value();

    
    ldmx_log(debug) 
        << "Track Index::" << GoodResult - 1 << std::endl
        << "===============" << std::endl
        << "ckf_result:: filtered " << ckf_result.filtered << std::endl
        << "ckf_result:: smoothed " << ckf_result.smoothed << std::endl
        << "ckf_result:: iSmoothed " << ckf_result.iSmoothed << std::endl
        << "ckf_result:: finished " << ckf_result.finished << std::endl
        << "Size of the active Tips " << ckf_result.activeTips.size() << std::endl
        << "Last Measurement indices "
        << ckf_result.lastMeasurementIndices.size();
    for (auto& lm : ckf_result.lastMeasurementIndices) {
      ldmx_log(debug)<< "LastMeasurementIndex=" << lm;
    }
    ldmx_log(debug) << "Last Track indices " << ckf_result.lastTrackIndices.size();
    
    for (auto& lt : ckf_result.lastTrackIndices) {
      ldmx_log(debug) << "LastTrackIndex=" << lt;
    }
    
    
    // The track tips are the last measurement index
    Acts::MultiTrajectory mj = ckf_result.fittedStates;

    // In the current case I should have a single trackTip
    // Check
    // https://github.com/acts-project/acts/blob/8f1f47bb57044b3e476d01b3dbafb13030038bd5/Examples/Io/Performance/ActsExamples/Io/Performance/TrackFitterPerformanceWriter.cpp#L114
    auto trackTip = ckf_result.lastMeasurementIndices.front();

    // Collect the trajectory summary info
    auto trajState =
        Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);

    // Check some track info

    ldmx_log(debug)
        << "nStates=" << trajState.nStates << std::endl
        << "nMeasurements=" << trajState.nMeasurements << std::endl
        << "nOutliers=" << trajState.nOutliers << std::endl
        << "nHoles=" << trajState.nHoles << std::endl
        << "chi2sum=" << trajState.chi2Sum << std::endl
        << "NDF=" << trajState.NDF << std::endl
        << "nsharedHits=" << trajState.nSharedHits;
        
        
    if (trajState.nStates !=
        trajState.nMeasurements + trajState.nOutliers + trajState.nHoles) {
      ldmx_log(warn)<<"Found track with nStates inconsistent with expectation";
      // continue;
    }

    // Cut on number of hits?
    if (trajState.nMeasurements < minHits_) continue;

    h_nHits_->Fill(trajState.nMeasurements);

    for (const auto& pair : ckf_result.fittedParameters) {
      // std::cout<<"Number of hits-on-track::" << (int) pair.first <<
      // std::endl;

      double p = pair.second.absoluteMomentum();

      double px = pair.second.momentum()(0);
      double py = pair.second.momentum()(1);
      double pz = pair.second.momentum()(2);
      double pt = sqrt(py * py + pz * pz);

      double truth_pt = sqrt(startParameters.at(0).momentum()(1) *
                                 startParameters.at(0).momentum()(1) +
                             startParameters.at(0).momentum()(2) *
                                 startParameters.at(0).momentum()(2));

      double resp = pair.second.absoluteMomentum() -
                    startParameters.at(0).absoluteMomentum();

      double respx = px - startParameters.at(0).momentum()(0);
      double respy = py - startParameters.at(0).momentum()(1);
      double respz = pz - startParameters.at(0).momentum()(2);

      double respt = pt - truth_pt;

      double resd0 =
          pair.second.get<Acts::BoundIndices::eBoundLoc0>() -
          startParameters.at(0).get<Acts::BoundIndices::eBoundLoc0>();
      double resz0 =
          pair.second.get<Acts::BoundIndices::eBoundLoc1>() -
          startParameters.at(0).get<Acts::BoundIndices::eBoundLoc1>();
      double resphi =
          pair.second.get<Acts::BoundIndices::eBoundPhi>() -
          startParameters.at(0).get<Acts::BoundIndices::eBoundPhi>();
      double restheta =
          pair.second.get<Acts::BoundIndices::eBoundTheta>() -
          startParameters.at(0).get<Acts::BoundIndices::eBoundTheta>();

      histo_p_->Fill(resp);
      histo_px_->Fill(respx);
      histo_py_->Fill(respy);
      histo_pz_->Fill(respz);
      histo_pt_->Fill(respt);

      histo_py_vs_p_->Fill(p, respy);
      histo_py_vs_py_->Fill(abs(py), respy);
      histo_pz_vs_p_->Fill(p, respz);
      histo_pz_vs_pz_->Fill(abs(pz), respz);
      histo_pt_vs_pt_->Fill(pt, respt);
      histo_pt_vs_p_->Fill(p, respt);

      histo_d0_->Fill(resd0);
      histo_z0_->Fill(resz0);
      histo_phi_->Fill(resphi);
      histo_theta_->Fill(restheta);

      h_p_->Fill(p);
      h_px_->Fill(px);
      h_py_->Fill(py);
      h_pz_->Fill(pz);
      h_pt_->Fill(pt);

      h_d0_->Fill(pair.second.get<Acts::BoundIndices::eBoundLoc0>());
      h_z0_->Fill(pair.second.get<Acts::BoundIndices::eBoundLoc1>());
      h_phi_->Fill(pair.second.get<Acts::BoundIndices::eBoundPhi>());
      h_theta_->Fill(pair.second.get<Acts::BoundIndices::eBoundTheta>());
      h_qop_->Fill(pair.second.get<Acts::BoundIndices::eBoundQOverP>());

      // auto cov = pair.second.covariance();
      // auto stddev = cov.diagonal().cwiseSqrt().eval();

      auto cov = pair.second.covariance();
      double sigma_d0 = sqrt(cov.value()(Acts::BoundIndices::eBoundLoc0,
                                         Acts::BoundIndices::eBoundLoc0));
      double sigma_z0 = sqrt(cov.value()(Acts::BoundIndices::eBoundLoc1,
                                         Acts::BoundIndices::eBoundLoc1));
      double sigma_phi = sqrt(cov.value()(Acts::BoundIndices::eBoundPhi,
                                          Acts::BoundIndices::eBoundPhi));
      double sigma_theta = sqrt(cov.value()(Acts::BoundIndices::eBoundTheta,
                                            Acts::BoundIndices::eBoundTheta));
      double sigma_qop = sqrt(cov.value()(Acts::BoundIndices::eBoundQOverP,
                                          Acts::BoundIndices::eBoundQOverP));

      h_d0_err_->Fill(sigma_d0);
      h_z0_err_->Fill(sigma_z0);
      h_phi_err_->Fill(sigma_phi);
      h_theta_err_->Fill(sigma_theta);
      h_qop_err_->Fill(sigma_qop);

      histo_d0_pull_->Fill(resd0 / sigma_d0);
      histo_z0_pull_->Fill(resz0 / sigma_z0);
      histo_phi_pull_->Fill(resphi / sigma_phi);
      histo_theta_pull_->Fill(restheta / sigma_theta);

      double sigma_p = pair.second.absoluteMomentum() *
                       pair.second.absoluteMomentum() * sigma_qop;

      h_p_err_->Fill(sigma_p);
      histo_p_pull_->Fill(resp / sigma_p);
    }

    // Create a track object

    ldmx::Track trk = ldmx::Track();
    trk.setPerigeeLocation(extr_surface->center(gctx_)[0],
                           extr_surface->center(gctx_)[1],
                           extr_surface->center(gctx_)[2]);

    trk.setChi2(trajState.chi2Sum);
    trk.setNhits(trajState.nMeasurements);
    trk.setNdf(trajState.NDF);
    trk.setNsharedHits(trajState.nSharedHits);

    // Set the perigee parameters (TODO:: There should only be one single track
    // per finding-fit for now. This method needs to be updated)
    // trk.setPerigeeParameters(ckf_result.fittedParameters.begin()->second.parameters());

    Acts::BoundVector tgt_srf_pars =
        ckf_result.fittedParameters.begin()->second.parameters();
    Acts::BoundMatrix tgt_srf_cov =
        ckf_result.fittedParameters.begin()->second.covariance()
            ? ckf_result.fittedParameters.begin()->second.covariance().value()
            : Acts::BoundSymMatrix::Identity();
    
    /*
    if (ckf_result.fittedParameters.begin()->second.charge() > 0) {
      std::cout<<getName()<<"::ERROR!!! ERROR!! Found track with q>0.
    Chi2="<<trajState.chi2Sum<<std::endl; mj.visitBackwards(trackTip, [&](const
    auto& state) { std::cout<<"Printing smoothed states"<<std::endl;
        std::cout<<state.smoothed()<<std::endl;
        std::cout<<"Printing filtered states"<<std::endl;
        std::cout<<state.filtered()<<std::endl;
      });


    }*/

    std::vector<double> v_tgt_srf_pars(
        tgt_srf_pars.data(),
        tgt_srf_pars.data() + tgt_srf_pars.rows() * tgt_srf_pars.cols());
    std::vector<double> v_tgt_srf_cov_flat;
    tracking::sim::utils::flatCov(tgt_srf_cov, v_tgt_srf_cov_flat);
    Acts::Vector3 trk_momentum =
        ckf_result.fittedParameters.begin()->second.momentum();
    Acts::Vector3 trk_position =
        ckf_result.fittedParameters.begin()->second.position(gctx_);

    trk.setPerigeeParameters(v_tgt_srf_pars);
    trk.setPerigeeCov(v_tgt_srf_cov_flat);
    trk.setMomentum(trk_momentum(0), trk_momentum(1), trk_momentum(2));
    trk.setPosition(trk_position(0), trk_position(1), trk_position(2));

    tracks.push_back(trk);
    ntracks_++;

    // Write the event display for the recoil
    if (ckf_result.fittedParameters.begin()->second.absoluteMomentum() < 1.2 &&
        false) {
      writeEvent(event, ckf_result.fittedParameters.begin()->second, mj,
                 trackTip, ldmxsps);
    }

    // Refit the track with the KalmanFitter using backward propagation
    if (kfRefit_) {
      std::cout << "Preparing theKF refit" << std::endl;
      std::vector<std::reference_wrapper<const ActsExamples::IndexSourceLink>>
          fit_trackSourceLinks;
      mj.visitBackwards(trackTip, [&](const auto& state) {
        const auto& sourceLink =
            static_cast<const ActsExamples::IndexSourceLink&>(
                state.uncalibrated());
        auto typeFlags = state.typeFlags();
        if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
          fit_trackSourceLinks.push_back(std::cref(sourceLink));
        }
      });

      std::cout << "Getting the logger and adding the extensions." << std::endl;
      const auto kfLogger =
          Acts::getDefaultLogger("KalmanFitter", Acts::Logging::INFO);
      Acts::KalmanFitterExtensions kfitter_extensions;
      kfitter_extensions.calibrator
          .connect<&LdmxMeasurementCalibrator::calibrate_1d>(&calibrator);
      kfitter_extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(
          &kfUpdater);
      kfitter_extensions.smoother
          .connect<&Acts::GainMatrixSmoother::operator()>(&kfSmoother);

      // rFiltering is true, so it should run in reversed direction.
      Acts::KalmanFitterOptions kfitter_options = Acts::KalmanFitterOptions(
          gctx_, bctx_, cctx_, kfitter_extensions,
          Acts::LoggerWrapper{*kfLogger}, propagator_options, &(*extr_surface),
          true, true, true);  // mScattering, exoLoss, rFiltering

      // Acts::MagneticFieldProvider::Cache cache =
      // sp_interpolated_bField_->makeCache(bctx_); std::cout<<"
      // BField::\n"<<sp_interpolated_bField_->getField(ckf_result.fittedParameters.begin()->second.position(gctx_),cache).value()
      // / Acts::UnitConstants::T <<std::endl;

      auto kf_refit_result = kf_->fit(
          fit_trackSourceLinks.begin(), fit_trackSourceLinks.end(),
          ckf_result.fittedParameters.begin()->second, kfitter_options);

      if (!kf_refit_result.ok()) {
        std::cout << "KF Refit failed" << std::endl;
      } else {
        auto kf_refit_value = kf_refit_result.value();
        auto kf_params = kf_refit_value.fittedParameters;
        h_p_refit_->Fill((*kf_params).absoluteMomentum());
        h_d0_refit_->Fill((*kf_params).get<Acts::BoundIndices::eBoundLoc0>());
        h_z0_refit_->Fill((*kf_params).get<Acts::BoundIndices::eBoundLoc1>());
        h_phi_refit_->Fill((*kf_params).get<Acts::BoundIndices::eBoundPhi>());
        h_theta_refit_->Fill(
            (*kf_params).get<Acts::BoundIndices::eBoundTheta>());
      }

    }  // Run the refit

    // Refit track using the GSF
    if (gsfRefit_) {
      try {
        const auto gsfLogger =
            Acts::getDefaultLogger("GSF", Acts::Logging::INFO);
        std::vector<std::reference_wrapper<const ActsExamples::IndexSourceLink>>
            fit_trackSourceLinks;
        mj.visitBackwards(trackTip, [&](const auto& state) {
          auto typeFlags = state.typeFlags();
          if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
            const auto& sourceLink =
                static_cast<const ActsExamples::IndexSourceLink&>(
                    state.uncalibrated());
            fit_trackSourceLinks.push_back(std::cref(sourceLink));
          }
        });

        // Same extensions of the KF
        Acts::GsfExtensions gsf_extensions;
        gsf_extensions.calibrator
            .connect<&LdmxMeasurementCalibrator::calibrate_1d>(&calibrator);
        gsf_extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(
            &kfUpdater);

        Acts::GsfOptions gsf_options{gctx_,
                                     bctx_,
                                     cctx_,
                                     gsf_extensions,
                                     Acts::LoggerWrapper{*gsfLogger},
                                     propagator_options,
                                     &(*extr_surface)};

        gsf_options.abortOnError = false;
        gsf_options.maxComponents = 4;
        gsf_options.disableAllMaterialHandling = false;

        auto gsf_refit_result =
            gsf_->fit(fit_trackSourceLinks.begin(), fit_trackSourceLinks.end(),
                      ckf_result.fittedParameters.begin()->second, gsf_options);

        if (!gsf_refit_result.ok()) {
          std::cout << "GSF Refit failed" << std::endl;
        } else {
          auto gsf_refit_value = gsf_refit_result.value();
          auto gsf_params = gsf_refit_value.fittedParameters;
          h_p_gsf_refit_->Fill((*gsf_params).absoluteMomentum());
          h_d0_gsf_refit_->Fill(
              (*gsf_params).get<Acts::BoundIndices::eBoundLoc0>());
          h_z0_gsf_refit_->Fill(
              (*gsf_params).get<Acts::BoundIndices::eBoundLoc1>());
          h_phi_gsf_refit_->Fill(
              (*gsf_params).get<Acts::BoundIndices::eBoundPhi>());
          h_theta_gsf_refit_->Fill(
              (*gsf_params).get<Acts::BoundIndices::eBoundTheta>());
        }

      } catch (...) {
        std::cout << "ERROR:: GSF Refit failed" << std::endl;
      }
    }  // do refit GSF
  }    // loop on CKF Results

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

void CKFProcessor::onProcessEnd() {
  std::cout << "Producer " << getName() << " found " << ntracks_
            << " tracks  / " << nseeds_ << " nseeds" << std::endl;

  TFile* outfile_ = new TFile((getName() + ".root").c_str(), "RECREATE");
  outfile_->cd();

  histo_p_->Write();
  histo_px_->Write();
  histo_py_->Write();
  histo_pz_->Write();
  histo_pt_->Write();

  histo_d0_->Write();
  histo_z0_->Write();
  histo_phi_->Write();
  histo_theta_->Write();

  histo_p_pull_->Write();
  histo_d0_pull_->Write();
  histo_z0_pull_->Write();
  histo_phi_pull_->Write();
  histo_theta_pull_->Write();

  h_p_->Write();
  h_px_->Write();
  h_py_->Write();
  h_pz_->Write();
  h_pt_->Write();

  histo_py_vs_p_->Write();
  histo_py_vs_py_->Write();
  histo_pz_vs_p_->Write();
  histo_pz_vs_pz_->Write();
  histo_pt_vs_pt_->Write();
  histo_pt_vs_p_->Write();

  h_d0_->Write();
  h_z0_->Write();
  h_phi_->Write();
  h_theta_->Write();
  h_qop_->Write();
  h_nHits_->Write();

  h_p_err_->Write();
  h_d0_err_->Write();
  h_z0_err_->Write();
  h_phi_err_->Write();
  h_theta_err_->Write();
  h_qop_err_->Write();

  h_p_refit_->Write();
  h_d0_refit_->Write();
  h_z0_refit_->Write();
  h_phi_refit_->Write();
  h_theta_refit_->Write();

  h_p_gsf_refit_->Write();
  h_d0_gsf_refit_->Write();
  h_z0_gsf_refit_->Write();
  h_phi_gsf_refit_->Write();
  h_theta_gsf_refit_->Write();
  // h_nHits_gsf_refit_->Write();

  h_p_truth_->Write();
  h_px_truth_->Write();
  h_py_truth_->Write();
  h_pz_truth_->Write();
  h_pt_truth_->Write();

  h_d0_truth_->Write();
  h_z0_truth_->Write();
  h_phi_truth_->Write();
  h_theta_truth_->Write();
  h_qop_truth_->Write();

  outfile_->Close();
  delete outfile_;

  std::cout << "PROCESSOR:: " << this->getName()
            << "   AVG Time/Event: " << processing_time_ / nevents_ << " ms"
            << std::endl;

  std::cout << "Breakdown::" << std::endl;
  std::cout << "setup       Avg Time/Event = "
            << profiling_map_["setup"] / nevents_ << " ms" << std::endl;
  std::cout << "hits        Avg Time/Event = "
            << profiling_map_["hits"] / nevents_ << " ms" << std::endl;
  std::cout << "seeds       Avg Time/Event = "
            << profiling_map_["seeds"] / nevents_ << " ms" << std::endl;
  std::cout << "cf_setup    Avg Time/Event = "
            << profiling_map_["ckf_setup"] / nevents_ << " ms" << std::endl;
  std::cout << "ckf_run     Avg Time/Event = "
            << profiling_map_["ckf_run"] / nevents_ << " ms" << std::endl;
  std::cout << "result_loop Avg Time/Event = "
            << profiling_map_["result_loop"] / nevents_ << " ms" << std::endl;
}

void CKFProcessor::configure(framework::config::Parameters& parameters) {
  dumpobj_ = parameters.getParameter<int>("dumpobj", 0);
  pionstates_ = parameters.getParameter<int>("pionstates", 0);
  steps_outfile_path_ = parameters.getParameter<std::string>(
      "steps_file_path", "propagation_steps.root");
  trackID_ = parameters.getParameter<int>("trackID", -1);
  pdgID_ = parameters.getParameter<int>("pdgID", 11);

  bfield_ = parameters.getParameter<double>("bfield", 0.);
  const_b_field_ = parameters.getParameter<bool>("const_b_field", true);
  bfieldMap_ = parameters.getParameter<std::string>(
      "bfieldMap_",
      "/Users/pbutti/sw/data_ldmx/"
      "BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat");
  propagator_step_size_ =
      parameters.getParameter<double>("propagator_step_size", 200.);
  propagator_maxSteps_ =
      parameters.getParameter<int>("propagator_maxSteps", 10000);
  perigee_location_ = parameters.getParameter<std::vector<double>>(
      "perigee_location", {0., 0., 0.});
  debug_ = parameters.getParameter<bool>("debug", false);
  hit_collection_ =
      parameters.getParameter<std::string>("hit_collection", "TaggerSimHits");

  removeStereo_ = parameters.getParameter<bool>("removeStereo", false);
  if (removeStereo_)
    std::cout << "CONFIGURE::removeStereo=" << (int)removeStereo_ << std::endl;

  use1Dmeasurements_ = parameters.getParameter<bool>("use1Dmeasurements", true);

  if (use1Dmeasurements_)
    std::cout << "CONFIGURE::use1Dmeasurements=" << (int)use1Dmeasurements_
              << std::endl;

  minHits_ = parameters.getParameter<int>("minHits", 7);

  std::cout << "CONFIGURE::minHits=" << minHits_ << std::endl;

  // Ckf specific options
  use_extrapolate_location_ =
      parameters.getParameter<bool>("use_extrapolate_location", true);
  extrapolate_location_ = parameters.getParameter<std::vector<double>>(
      "extrapolate_location", {0., 0., 0.});
  use_seed_perigee_ = parameters.getParameter<bool>("use_seed_perigee", false);

  // seeds from the event
  seed_coll_name_ =
      parameters.getParameter<std::string>("seed_coll_name", "seedTracks");

  // output track collection
  out_trk_collection_ =
      parameters.getParameter<std::string>("out_trk_collection", "Tracks");

  // Hit smearing

  do_smearing_ = parameters.getParameter<bool>("do_smearing", false);
  sigma_u_ = parameters.getParameter<double>("sigma_u", 0.01);
  sigma_v_ = parameters.getParameter<double>("sigma_v", 0.);

  std::cout << __PRETTY_FUNCTION__ << "  HitCollection::" << hit_collection_
            << std::endl;

  kfRefit_ = parameters.getParameter<bool>("kfRefit", false);
  gsfRefit_ = parameters.getParameter<bool>("gsfRefit", false);
}

void CKFProcessor::testField(
    const std::shared_ptr<Acts::MagneticFieldProvider> bfield,
    const Acts::Vector3& eval_pos) const {
  Acts::MagneticFieldProvider::Cache cache = bfield->makeCache(bctx_);
  std::cout << "Pos::\n" << eval_pos << std::endl;
  std::cout << " BField::\n"
            << bfield->getField(eval_pos, cache).value() /
                   Acts::UnitConstants::T
            << std::endl;
}

void CKFProcessor::testMeasurmentCalibrator(
    const LdmxMeasurementCalibrator& calibrator,
    const std::unordered_map<Acts::GeometryIdentifier,
                             std::vector<ActsExamples::IndexSourceLink>>& map)
    const {
  for (const auto& pair : map) {
    std::cout << "GeometryID::" << pair.first << std::endl;
    for (auto& sl : pair.second) {
      calibrator.test(gctx_, sl);
    }
  }
}

// This functioon takes the input parameters and makes the propagation for a
// simple event display
void CKFProcessor::writeEvent(
    framework::Event& event,
    const Acts::BoundTrackParameters& perigeeParameters,
    const Acts::MultiTrajectory& mj, const int& trackTip,
    const std::vector<ldmx::LdmxSpacePoint*> ldmxsps) {
  // Prepare the outputs..
  std::vector<std::vector<Acts::detail::Step>> propagationSteps;
  propagationSteps.reserve(1);

  /// Using some short hands for Recorded Material
  using RecordedMaterial = Acts::MaterialInteractor::result_type;

  /// And recorded material track
  /// - this is start:  position, start momentum
  ///   and the Recorded material
  using RecordedMaterialTrack =
      std::pair<std::pair<Acts::Vector3, Acts::Vector3>, RecordedMaterial>;

  /// Finally the output of the propagation test
  using PropagationOutput =
      std::pair<std::vector<Acts::detail::Step>, RecordedMaterial>;

  PropagationOutput pOutput;
  const auto evtLogger =
      Acts::getDefaultLogger("evtDisplay", Acts::Logging::INFO);
  Acts::PropagatorOptions<ActionList, AbortList> propagator_options(
      gctx_, bctx_, Acts::LoggerWrapper{*evtLogger});

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
  sLogger.sterile = false;
  // Set a maximum step size
  propagator_options.maxStepSize =
      propagator_step_size_ * Acts::UnitConstants::mm;
  propagator_options.maxSteps = propagator_maxSteps_;

  // Loop over the states and the surfaces of the multi-trajectory and get
  // the arcs of helix from start to next surface

  std::vector<Acts::BoundTrackParameters> prop_parameters;
  // std::vector<std::reference_wrapper<const Acts::Surface>> ref_surfaces;
  std::vector<std::reference_wrapper<const ActsExamples::IndexSourceLink>>
      sourceLinks;

  mj.visitBackwards(trackTip, [&](const auto& state) {
    auto typeFlags = state.typeFlags();

    // Only store the track states for each measurement
    if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
      const auto& surface = state.referenceSurface();
      // ref_surfaces.push_back(surface);
      Acts::BoundVector smoothed = state.smoothed();
      // auto cov = state.smoothedCovariance;

      Acts::ActsScalar q = smoothed[Acts::eBoundQOverP] < 0
                               ? -1 * Acts::UnitConstants::e
                               : Acts::UnitConstants::e;

      prop_parameters.push_back(
          Acts::BoundTrackParameters(surface.getSharedPtr(), smoothed, q));
    }
  });

  // Reverse the parameters to start from the target
  std::reverse(prop_parameters.begin(), prop_parameters.end());

  // This holds all the steps to be merged
  std::vector<std::vector<Acts::detail::Step>> tmpSteps;
  tmpSteps.reserve(prop_parameters.size());

  
  // Start from the first parameters
  // Propagate to next surface
  // Grab the next parameters
  // Propagate to the next surface..
  // The last parameters just propagate
  // TODO Fix double code
  // TODO Fix contorted code - directly push to the same vector

  std::vector<Acts::detail::Step> steps;

  // compute first the perigee to first surface:
  auto result = propagator_->propagate(perigeeParameters,
                                       prop_parameters.at(0).referenceSurface(),
                                       propagator_options);

  if (result.ok()) {
    const auto& resultValue = result.value();
    auto steppingResults =
        resultValue.template get<Acts::detail::SteppingLogger::result_type>();
    // Set the stepping result
    pOutput.first = std::move(steppingResults.steps);

    for (auto& step : pOutput.first) steps.push_back(std::move(step));
  }

  // follow now the trajectory

  for (int i_params = 0; i_params < prop_parameters.size(); i_params++) {
    if (i_params < prop_parameters.size() - 1) {
      auto result = propagator_->propagate(
          prop_parameters.at(i_params),
          prop_parameters.at(i_params + 1).referenceSurface(),
          propagator_options);

      if (result.ok()) {
        const auto& resultValue = result.value();
        auto steppingResults =
            resultValue
                .template get<Acts::detail::SteppingLogger::result_type>();
        // Set the stepping result
        pOutput.first = std::move(steppingResults.steps);

        for (auto& step : pOutput.first) steps.push_back(std::move(step));

        // Record the propagator steps
        // tmpSteps.push_back(std::move(pOutput.first));
      }
    }

    // propagation for the last state
    else {
      auto result = propagator_->propagate(prop_parameters.at(i_params),
                                           propagator_options);

      if (result.ok()) {
        const auto& resultValue = result.value();
        auto steppingResults =
            resultValue
                .template get<Acts::detail::SteppingLogger::result_type>();
        // Set the stepping result
        pOutput.first = std::move(steppingResults.steps);

        for (auto& step : pOutput.first) steps.push_back(std::move(step));

        // Record the propagator steps
        // tmpSteps.push_back(std::move(pOutput.first));
      }
    }
  }

  // This holds all the steps to be merged
  // TODO Remove this and put all in the same vector directly in the for loop
  // above

  /*

  int totalSize=0;

  for (auto & steps: tmpSteps)
    totalSize+=steps.size();

    allSteps.reserve(totalSize);

  for (auto & steps: tmpSteps)
    allSteps.insert(allSteps.end(), steps.begin(), steps.end());
  */

  propagationSteps.push_back(steps);
  Acts::Vector3 gen_pos(0., 0., 0.);
  Acts::Vector3 gen_mom(0., 0., 0.);
  writer_->WriteSteps(event, propagationSteps, ldmxsps, gen_pos, gen_mom);
}

auto CKFProcessor::makeGeoIdSourceLinkMap(
    const std::vector<ldmx::LdmxSpacePoint*>& ldmxsps)
    -> std::unordered_multimap<Acts::GeometryIdentifier,
                               ActsExamples::IndexSourceLink> {
  std::unordered_multimap<Acts::GeometryIdentifier,
                          ActsExamples::IndexSourceLink>
      geoId_sl_map;

  // Check the hits associated to the surfaces
  for (unsigned int i_ldmx_hit = 0; i_ldmx_hit < ldmxsps.size(); i_ldmx_hit++) {
    ldmx::LdmxSpacePoint* ldmxsp = ldmxsps.at(i_ldmx_hit);
    unsigned int layerid = ldmxsp->layer();

    // const Acts::Surface* hit_surface = layer_surface_map_.at(layerid);

    const Acts::Surface* hit_surface = ldmx_tg->getSurface(layerid);

    if (hit_surface) {
      // Transform the ldmx space point from global to local and store the
      // information

      Acts::Vector3 dummy_momentum;
      Acts::Vector2 local_pos;
      try {
        local_pos = hit_surface
                        ->globalToLocal(gctx_, ldmxsp->global_pos_,
                                        dummy_momentum, 0.320)
                        .value();
      } catch (const std::exception& e) {
        std::cout << "WARNING:: hit not on surface.. Skipping." << std::endl;
        std::cout << ldmxsp->global_pos_ << std::endl;
        continue;
      }

      // Smear the local position

      if (do_smearing_) {
        float smear_factor{(*normal_)(generator_)};

        local_pos[0] += smear_factor * sigma_u_;
        smear_factor = (*normal_)(generator_);
        local_pos[1] += smear_factor * sigma_v_;
        // update covariance
        ldmxsp->setLocalCovariance(sigma_u_ * sigma_u_, sigma_v_ * sigma_v_);

        // cache the acts x coordinate
        double original_x = ldmxsp->global_pos_(0);

        // transform to global
        ldmxsp->global_pos_ =
            hit_surface->localToGlobal(gctx_, local_pos, dummy_momentum);

        // update the acts x location
        ldmxsp->global_pos_(0) = original_x;

      }

      ldmxsp->local_pos_ = local_pos;

      ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(),
                                           i_ldmx_hit);

      geoId_sl_map.insert(std::make_pair(hit_surface->geometryId(), idx_sl));

    } else
      std::cout << getName() << "::HIT " << i_ldmx_hit << " at layer"
                << (ldmxsps.at(i_ldmx_hit))->layer()
                << " is not associated to any surface?!" << std::endl;
  }

  return geoId_sl_map;
}

auto CKFProcessor::makeLdmxSpacepoints(
    const std::vector<ldmx::SimTrackerHit>& sim_hits)
    -> std::vector<ldmx::LdmxSpacePoint*> {
  std::vector<ldmx::LdmxSpacePoint*> ldmxsps;

  // Convert to ldmxsps
  for (auto& simHit : sim_hits) {
    // Remove low energy deposit hits
    if (simHit.getEdep() > 0.05) {
      // Only selects hits that have trackID==1
      if (trackID_ > 0 && simHit.getTrackID() != trackID_) continue;

      if (pdgID_ != -9999 && abs(simHit.getPdgID()) != pdgID_) continue;

      ldmx::LdmxSpacePoint* ldmxsp =
          utils::convertSimHitToLdmxSpacePoint(simHit);

      if (removeStereo_) {
        unsigned int layerid = ldmxsp->layer();
        if (layerid == 3101 || layerid == 3201 || layerid == 3301 ||
            layerid == 3401)
          continue;
      }

      ldmxsps.push_back(ldmxsp);
    }
  }

  return ldmxsps;
}

// This is used to propagate the initial eN states through the detector

void CKFProcessor::propagateENstates(framework::Event& event,
                                     std::string inputFile,
                                     std::string outFile) {
  std::ifstream inFile(inputFile);
  if (!inFile.is_open()) {
    std::cout << __PRETTY_FUNCTION__ << " could not read input file "
              << inputFile << std::endl;
    return;
  }

  // skip first line
  std::string spdg, smass, sE, spx, spy, spz, sp;
  int pdg;
  float mass, E, px, py, pz, p;
  std::getline(inFile, spdg);

  Acts::Vector3 gen_pos{0., 0., 0.};
  Acts::Vector3 gen_mom{0., 0., 0.};

  int total = 0;
  int pzCut = 0;
  int pCut = 0;
  int success = 0;
  int fail = 0;

  std::default_random_engine generator;
  std::uniform_real_distribution<double> bY(-40, 40);
  std::uniform_real_distribution<double> bX(-10, 10);

  // Generate uniform pions

  std::uniform_real_distribution<double> PX(-4, 4);
  std::uniform_real_distribution<double> PY(-4, 4);
  std::uniform_real_distribution<double> PZ(0.0, 4);

  std::uniform_real_distribution<double> P(0.05, 4);
  std::uniform_real_distribution<double> THETA(0, 1.57079632679);
  std::uniform_real_distribution<double> PHI(0.0, 6.28318530718);

  // Loop
  // while(inFile>>pdg >> mass >> E >> px >> py >> pz >> p) {
  for (int i_pion = 0; i_pion < pionstates_; i_pion++) {
    p = P(generator);
    double theta = THETA(generator);
    double phi = PHI(generator);
    // px = PX(generator);
    // py = PY(generator);
    // pz = PZ(generator);
    // p = sqrt(px*px + py*py + pz*pz);

    px = p * cos(theta);
    py = p * sin(theta) * cos(phi);
    pz = p * sin(theta) * sin(phi);

    total += 1;
    std::vector<std::vector<Acts::detail::Step>> propagationSteps;

    // randomize beamspot
    // global Y
    double by = bY(generator);

    // global X
    double bx = bX(generator);

    // Already rotated in tracking frame
    gen_pos(0) = 0.;
    gen_pos(1) = bx;
    gen_pos(2) = by;

    // Skip particles that are not propagated in the forward direction from the
    // target.
    if (px < 0) {
      pzCut++;
      continue;
    }

    // Skip particles with a momentum too low to be reconstructed

    if (p < 0.05) {
      pCut++;
      continue;
    }

    // std::cout<<pdg <<" " << mass <<" " << E <<" " << px <<" " <<py<<" "
    // <<pz<<" " <<p<<std::endl;

    // Transform to MeV because that's what TrackUtils assumes
    gen_mom(0) = px / Acts::UnitConstants::MeV;
    gen_mom(1) = py / Acts::UnitConstants::MeV;
    gen_mom(2) = pz / Acts::UnitConstants::MeV;
    // gen_pos = tracking::sim::utils::Ldmx2Acts(gen_pos);
    // gen_mom = tracking::sim::utils::Ldmx2Acts(gen_mom);

    Acts::ActsScalar q = -1 * Acts::UnitConstants::e;

    if (pdg == 211 || pdg == 2212) q = +1 * Acts::UnitConstants::e;

    Acts::FreeVector part_free =
        tracking::sim::utils::toFreeParameters(gen_pos, gen_mom, q);

    // perigee on the track
    std::shared_ptr<const Acts::PerigeeSurface> gen_surface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
            part_free[Acts::eFreePos0], part_free[Acts::eFreePos1],
            part_free[Acts::eFreePos2]));

    // std::cout<<"gen_free"<<std::endl;
    // std::cout<<part_free<<std::endl;

    // Transform the free parameters to the bound parameters
    auto bound_params = Acts::detail::transformFreeToBoundParameters(
                            part_free, *gen_surface, gctx_)
                            .value();

    using RecordedMaterial = Acts::MaterialInteractor::result_type;
    using RecordedMaterialTrack =
        std::pair<std::pair<Acts::Vector3, Acts::Vector3>, RecordedMaterial>;
    using PropagationOutput =
        std::pair<std::vector<Acts::detail::Step>, RecordedMaterial>;

    PropagationOutput pOutput;
    const auto eNLogger = Acts::getDefaultLogger("eN", Acts::Logging::INFO);

    Acts::PropagatorOptions<ActionList, AbortList> propagator_options(
        gctx_, bctx_, Acts::LoggerWrapper{*eNLogger});
    propagator_options.pathLimit = std::numeric_limits<double>::max();
    propagator_options.loopProtection = false;
    auto& mInteractor =
        propagator_options.actionList.get<Acts::MaterialInteractor>();
    mInteractor.multipleScattering = true;
    mInteractor.energyLoss = true;
    mInteractor.recordInteractions = false;

    auto& sLogger =
        propagator_options.actionList.get<Acts::detail::SteppingLogger>();
    sLogger.sterile = false;
    propagator_options.maxStepSize = 5 * Acts::UnitConstants::mm;
    propagator_options.maxSteps = 2000;

    Acts::BoundTrackParameters startParameters(
        gen_surface, std::move(bound_params), std::move(std::nullopt));
    auto result = propagator_->propagate(startParameters, propagator_options);

    if (result.ok()) {
      const auto& resultValue = result.value();
      auto steppingResults =
          resultValue.template get<Acts::detail::SteppingLogger::result_type>();
      pOutput.first = std::move(steppingResults.steps);
      propagationSteps.push_back(std::move(pOutput.first));

      success += 1;
    } else {
      // std::cout<<"PF::ERROR::PROPAGATION RESULTS ARE NOT OK!!"<<std::endl;
      fail += 1;
    }

    // empty
    std::vector<ldmx::LdmxSpacePoint*> hits{};
    writer_->WriteSteps(event, propagationSteps, hits, gen_pos, gen_mom);

  }  // loop on inputs
  std::cout << "Total=" << total << " success=" << success << " fail=" << fail
            << " pzCut=" << pzCut << " pCut=" << pCut << std::endl;
}

}  // namespace sim
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::sim, CKFProcessor)
