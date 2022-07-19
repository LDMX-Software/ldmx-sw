#include "Tracking/Sim/TrackingGeometryMaker.h"
#include "Tracking/Sim/GeometryContainers.h"

#include "SimCore/Event/SimParticle.h"

//--- ACTS ---//
#include "Acts/Plugins/TGeo/TGeoPrimitivesHelper.hpp"

//--- DD4Hep ---//
#include "DD4hep/DetElement.h"

//--- C++ StdLib ---//
#include <iostream>
#include <algorithm> //std::vector reverse

namespace tracking {
namespace sim {
    
TrackingGeometryMaker::TrackingGeometryMaker(const std::string &name,
                                             framework::Process &process)
    : framework::Producer(name, process) {

  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);
}

TrackingGeometryMaker::~TrackingGeometryMaker() {}

void TrackingGeometryMaker::onProcessStart() {


  profiling_map_["setup"]        = 0.;
  profiling_map_["hits"]         = 0.;
  profiling_map_["seeds"]        = 0.;
  profiling_map_["ckf_setup"]    = 0.;
  profiling_map_["ckf_run"]      = 0.;
  profiling_map_["result_loop"]  = 0.;
  

  detector_ = &detector();
  gctx_ = Acts::GeometryContext();
  bctx_ = Acts::MagneticFieldContext();


  // Get the random seed service
  //auto rseed{getCondition<framework::RandomNumberSeedService>(
  //    framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME)};
    
  // Create a seed and update the generator with it
  //generator_.seed(rseed.getSeed(getName()));
  //generator_.seed(std::chrono::system_clock::now().time_since_epoch().count());
  generator_.seed(1);

  // Get the world detector element
  dd4hep::DetElement world{detector_->world()};
  Acts::CuboidVolumeBuilder cvb;
  std::vector<dd4hep::DetElement> subdetectors;

  //Get the ACTS Logger
  
  auto loggingLevel = Acts::Logging::INFO;
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("DD4hepConversion", loggingLevel));
    
  //The subdetectors should be the TaggerTracker and the Recoil Tracker
  
  collectSubDetectors_dd4hep(world,subdetectors);
  if (debug_)
    std::cout<<"PF::DEBUG::"<<__PRETTY_FUNCTION__<<" size  of subdetectors::"<<subdetectors.size()<<std::endl;
  
  //loop over the subDetectors to gather all the configurations
  
  std::vector<Acts::CuboidVolumeBuilder::VolumeConfig> volBuilderConfigs;
  for (auto& subDetector : subdetectors) {
    if (debug_)
      std::cout<<"PF::DEBUG:: Translating DD4Hep sub detector: " << subDetector.name()<<std::endl;
    //create the cuboid volume configurations for the builder
    volBuilderConfigs.push_back(volumeBuilder_dd4hep(subDetector,loggingLevel));
  }
  
  //Create the builder
  Acts::CuboidVolumeBuilder::Config config;
  config.position = {0., 0., 0.};
  config.length = {2000, 2000, 2000};
  config.volumeCfg = volBuilderConfigs;

  cvb.setConfig(config);
    
  Acts::TrackingGeometryBuilder::Config tgbCfg;
  tgbCfg.trackingVolumeBuilders.push_back(
      [=](const auto& cxt, const auto& inner, const auto&) {
        return cvb.trackingVolume(cxt, inner, nullptr);
      });
  
  Acts::TrackingGeometryBuilder tgb(tgbCfg);
  
  tGeometry_ = tgb.trackingGeometry(gctx_);

  
  //Move this to a function
  
  if (dumpobj_) {
       
    double outputScalor = 1.0;   ///< scale output values
    size_t outputPrecision = 6;  ///< floating point precision
       
    Acts::ObjVisualization3D objVis(outputPrecision, outputScalor);
       
    Acts::ViewConfig containerView = Acts::ViewConfig({220, 220, 220});
    Acts::ViewConfig volumeView = Acts::ViewConfig({220, 220, 0});
    Acts::ViewConfig sensitiveView = Acts::ViewConfig({0, 180, 240});
    Acts::ViewConfig passiveView = Acts::ViewConfig({240, 280, 0});
    Acts::ViewConfig gridView = Acts::ViewConfig({220, 0, 0});

    std::string outputDir="/Users/pbutti/sw/ldmx-sw/trackingGeo/";
    
    Acts::GeometryView3D::drawTrackingVolume(
        objVis, *(tGeometry_->highestTrackingVolume()),
        gctx_, containerView,
        volumeView, passiveView, sensitiveView, gridView,
        true,"",".");
  }


  //==> Move to a separate processor <== //

  //Generate a constant magnetic field
  Acts::Vector3 b_field(0., 0., bfield_ * Acts::UnitConstants::T);

  if (debug_) {
    std::cout<<"PF::DEBUG::BFIELD"<<std::endl;
    std::cout<<"=========="<<std::endl;
    std::cout<<b_field / Acts::UnitConstants::T<<std::endl;
    std::cout<<"=========="<<std::endl;
  }
  std::shared_ptr<Acts::ConstantBField> bField = std::make_shared<Acts::ConstantBField>(b_field);

  //TODO:: Move this to an external file
  auto localToGlobalBin_xyz = [](std::array<size_t, 3> bins,
                                 std::array<size_t, 3> sizes) {
    return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] + bins[2]);  //xyz - field space
    //return (bins[1] * (sizes[2] * sizes[0]) + bins[2] * sizes[0] + bins[0]);    //zxy
    
  };

  if (debug_) {
    std::cout<<"PF::BFIELDMAP"<<std::endl;
    std::cout<<bfieldMap_<<std::endl;
  }
  
  //CHECK:::Tests/IntegrationTests/PropagationTestsAtlasField.cpp
  InterpolatedMagneticField3 map = makeMagneticFieldMapXyzFromText(std::move(localToGlobalBin_xyz), bfieldMap_,
                                                                   1. * Acts::UnitConstants::mm, //default scale for axes length
                                                                   1000. * Acts::UnitConstants::T, //The map is in kT, so scale it to T
                                                                   false, //not symmetrical
                                                                   true //rotate the axes to tracking frame
                                                                   );

  InterpolatedMagneticField3 map_copy = makeMagneticFieldMapXyzFromText(std::move(localToGlobalBin_xyz), bfieldMap_,
                                                                        1. * Acts::UnitConstants::mm, //default scale for axes length
                                                                        1000. * Acts::UnitConstants::T, //The map is in kT, so scale it to T
                                                                        false, //not symmetrical
                                                                        true //rotate the axes to tracking frame
                                                                        );
  
  sp_interpolated_bField_      = std::make_shared<InterpolatedMagneticField3>(std::move(map));
  sp_interpolated_bField_copy_ = std::make_shared<InterpolatedMagneticField3>(std::move(map_copy));
  
  
  //Setup the navigator for KF
  Acts::Navigator::Config navCfg{tGeometry_};
  navCfg.resolveMaterial   = true;
  navCfg.resolvePassive    = true;
  navCfg.resolveSensitive  = true;
  Acts::Navigator navigator(navCfg);

  //Setup the navigator for GSF
  Acts::Navigator::Config gsf_navigator_cfg{tGeometry_};
  gsf_navigator_cfg.resolvePassive = false;
  gsf_navigator_cfg.resolveMaterial = true;
  gsf_navigator_cfg.resolveSensitive = true;
  Acts::Navigator gsf_navigator(gsf_navigator_cfg);

  
  //Setup the stepper (do a straight line first)
  //auto&& stepper = Acts::EigenStepper<>{std::move(bField)};
  

  auto&& stepper_const         = Acts::EigenStepper<>{std::move(bField)};
  auto&& stepper_interpolated  = Acts::EigenStepper<>{std::move(sp_interpolated_bField_)};

  auto&& gsf_stepper          = Acts::MultiEigenStepperLoop{std::move(sp_interpolated_bField_copy_)};
  
  //using Propagator = Acts::Propagator<Stepper, Acts::Navigator>;
    
  //Setup the propagator
  if (const_b_field_) {
    std::cout<<__PRETTY_FUNCTION__<<std::endl;
    std::cout<<"Using constant b-field"<<std::endl;
    propagator_ = std::make_shared<Propagator>(stepper_const, navigator);
  }
  else {
    propagator_ = std::make_shared<Propagator>(stepper_interpolated, navigator);
    std::cout<<__PRETTY_FUNCTION__<<std::endl;
    std::cout<<"Using interpolated B-Field Map"<<std::endl;
  }

  //Only works with interpolated
  Acts::Propagator gsf_propagator(std::move(gsf_stepper), std::move(gsf_navigator));

  //Setup the GSF Fitter
  //gsf_ = std::make_shared<Acts::GaussianSumFitter<Propagator> >(std::move(gsf_propagator));

  //Acts::GaussianSumFitter<Propagator> gsf(std::move(gsf_propagator));
  //Acts::GaussianSumFitter<decltype(gsf_propagator)> gsf(std::move(gsf_propagator));
  gsf_ = std::make_shared< Acts::GaussianSumFitter<decltype(gsf_propagator)> >(std::move(gsf_propagator));
  
  //Setup the propagator steps writer
  PropagatorStepWriter::Config cfg;
  cfg.filePath = steps_outfile_path_;

  writer_ = std::make_shared<PropagatorStepWriter>(cfg);

  
  //Create a mapping between the layers and the Acts::Surface
  makeLayerSurfacesMap(tGeometry_);


  //Prepare histograms

  //getHistoDirectory();

  //Track parameters
  //histograms_.create("d0_reco","d0_reco",100,-20,20);
  //histograms_.create("z0_reco","z0_reco",100,-20,20);
  //histograms_.create("q/p","q_over_p",100,-1,1);
  //histograms_.create("p","p",100,0,10);
  //histograms_.create("theta","theta",100,-3.15,3.15);
  //histograms_.create("phi","phi",100,-3.15,3.15);
  //histograms_.create("t","t",100,0,100);

  //Residuals
  //histograms_.create("d0_res","d0_reco",100,-5,5);
  //histograms_.create("z0_res","z0_reco",100,-5,5);
  //histograms_.create("q/p_res","q_over_p",100,-0.1,0.1);
  //histograms_.create("p_res","p",100,-2,2);
  //histograms_.create("theta_res","theta",100,-0.1,0.1);
  //histograms_.create("phi_res","phi",100,-0.1,0.1);
  //histograms_.create("t_res","t",100,-0.1,0.1);

  //Validation histograms
  
  histo_p_      = new TH1F("p_res",    "p_res",200,-1,1);
  histo_d0_     = new TH1F("d0_res",   "d0_res",200,-0.2,0.2);
  histo_z0_     = new TH1F("z0_res",   "z0_res",200,-0.75,0.75);
  histo_phi_    = new TH1F("phi_res",  "phi_res",200,-0.015,0.015);
  histo_theta_  = new TH1F("theta_res","theta_res",200,-0.01,0.01);
  histo_qop_    = new TH1F("qop_res","qop_res",200,-5,5);

  histo_p_pull_      = new TH1F("p_pull",    "p_pull",    200,-5,5);
  histo_d0_pull_     = new TH1F("d0_pull",   "d0_pull",   200,-5,5);
  histo_z0_pull_     = new TH1F("z0_pull",   "z0_pull",   200,-5,5);
  histo_phi_pull_    = new TH1F("phi_pull",  "phi_pull",  200,-5,5);
  histo_theta_pull_  = new TH1F("theta_pull","theta_pull",200,-5,5);
  histo_qop_pull_    = new TH1F("qop_pull",  "qop_pull",  200,-5,5);

  h_p_      = new TH1F("p",    "p",600,0,6);
  h_d0_     = new TH1F("d0",   "d0",100,-20,20);
  h_z0_     = new TH1F("z0",   "z0",100,-50,50);
  h_phi_    = new TH1F("phi",  "phi",200,-0.5,0.5);
  h_theta_  = new TH1F("theta","theta",200,0.8,2.2);
  h_qop_    = new TH1F("qop","qop",200,-10,10);
  h_nHits_  = new TH1F("nHits","nHits",15,0,15);

  h_p_err_      = new TH1F("p_err",    "p_err"    ,600,0,1);
  h_d0_err_     = new TH1F("d0_err",   "d0_err"   ,100,0,0.05);
  h_z0_err_     = new TH1F("z0_err",   "z0_err"   ,100,0,0.8);
  h_phi_err_    = new TH1F("phi_err",  "phi_err"  ,200,0,0.002);
  h_theta_err_  = new TH1F("theta_err","theta_err",200,0,0.02);
  h_qop_err_    = new TH1F("qop_err",  "qop_err"  ,200,0,0.1);

  h_p_refit_      = new TH1F("p_refit",     "p_refit",600,0,6);
  h_d0_refit_     = new TH1F("d0_refit",    "d0_refit",100,-20,20);
  h_z0_refit_     = new TH1F("z0_refit",    "z0_refit",100,-50,50);
  h_phi_refit_    = new TH1F("phi_refit",   "phi_refit",200,-0.5,0.5);
  h_theta_refit_  = new TH1F("theta_refit", "theta_refit",200,0.8,2.2);
  

  h_p_gsf_refit_      = new TH1F("p_gsf_refit",     "p_gsf_refit",600,0,6);
  h_d0_gsf_refit_     = new TH1F("d0_gsf_refit",    "d0_gsf_refit",100,-20,20);
  h_z0_gsf_refit_     = new TH1F("z0_gsf_refit",    "z0_gsf_refit",100,-50,50);
  h_phi_gsf_refit_    = new TH1F("phi_gsf_refit",   "phi_gsf_refit",200,-0.5,0.5);
  h_theta_gsf_refit_  = new TH1F("theta_gsf_refit", "theta_gsf_refit",200,0.8,2.2);

  h_p_gsf_refit_res_     = new TH1F("p_gsf_res", "p_gsf_res", 200,-1,1);
  h_qop_gsf_refit_res_   = new TH1F("qop_gsf_res", "qop_gsf_res", 200,-5,5);
  

  h_p_truth_      = new TH1F("p_truth",      "p_truth",600,0,6);
  h_d0_truth_     = new TH1F("d0_truth",     "d0_truth",100,-20,20);
  h_z0_truth_     = new TH1F("z0_truth_",    "z0_truth",100,-50,50);
  h_phi_truth_    = new TH1F("phi_truth",    "phi_truth",200,-0.5,0.5);
  h_theta_truth_  = new TH1F("theta_truth`", "theta_truth",200,0.8,2.2);
  h_qop_truth_    = new TH1F("qop_truth","qop_truth",200,-10,10);

  h_tgt_scoring_x_y_      = new TH2F("tgt_scoring_x_y",    "tgt_scoring_x_y",100,-40,40,100,-40,40);
  h_tgt_scoring_z_        = new TH1F("tgt_scoring_z",      "tgt_scoring_z"  ,100,0,10);
  
}

void TrackingGeometryMaker::produce(framework::Event &event) {


  //TODO use global variable instead and call clear;
  std::vector<ldmx::Track> tracks;
  
  auto start = std::chrono::high_resolution_clock::now();
  
  nevents_++;
  if (nevents_ % 1000 == 0)
    std::cout<<"events processed:"<<nevents_<<std::endl;
  
  if (debug_) {
    std::cout<<"PF ::DEBUG:: "<<__PRETTY_FUNCTION__<<std::endl;
    std::cout<<"Processing event "<<&event<<std::endl;
  }

  //1) Setup the actions and the abort list. For debugging add the Stepping Logger
  //2) Create the propagator options from them. If the magneticField context is empty it won't be used.
  //2a) If I want to pass a constant Field then use the Constant BField Class (inherits from the provider)
  
  //options_ = TestPropagatorOptions();
  
  //3) Get the steps vector and pass it to the root Writer -> in the event processor

  //Setup the starting point 

  std::shared_ptr<const Acts::PerigeeSurface> perigee_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(perigee_location_.at(0), perigee_location_.at(1), perigee_location_.at(2)));

  //Get the ACTS Logger -  Very annoying to have to define it in order to run this test.
  auto loggingLevel = Acts::Logging::DEBUG;
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("LDMX Tracking Goemetry Maker", loggingLevel));
  
  Acts::PropagatorOptions<ActionList, AbortList> propagator_options(gctx_, bctx_, Acts::LoggerWrapper{logger()});
  
  propagator_options.pathLimit = std::numeric_limits<double>::max();
  
  // Activate loop protection at some pt value
  propagator_options.loopProtection = false; //(startParameters.transverseMomentum() < cfg.ptLoopers);
  
  // Switch the material interaction on/off & eventually into logging mode
  auto& mInteractor = propagator_options.actionList.get<Acts::MaterialInteractor>();
  mInteractor.multipleScattering = true;
  mInteractor.energyLoss         = true;
  mInteractor.recordInteractions = false;
  
  // The logger can be switched to sterile, e.g. for timing logging
  auto& sLogger = propagator_options.actionList.get<Acts::detail::SteppingLogger>();
  sLogger.sterile = true;
  // Set a maximum step size
  propagator_options.maxStepSize = propagator_step_size_ * Acts::UnitConstants::mm;
  propagator_options.maxSteps    = propagator_maxSteps_;

  //Electron hypothesis
  propagator_options.mass = 0.511 * Acts::UnitConstants::MeV;
      
  //std::cout<<"Setting up the Kalman Filter Algorithm"<<std::endl;

  //#######################//
  //Kalman Filter algorithm//
  //#######################//
  
  //Step 1 - Form the source links
  
  //std::vector<ActsExamples::IndexSourceLink> sourceLinks;
  //a) Loop over the sim Hits

  auto setup = std::chrono::high_resolution_clock::now();
  profiling_map_["setup"] += std::chrono::duration<double,std::milli>(setup-start).count();
  
  
  const std::vector<ldmx::SimTrackerHit> sim_hits  = event.getCollection<ldmx::SimTrackerHit>(hit_collection_);
    
  std::vector<ldmx::LdmxSpacePoint* > ldmxsps;

  if (debug_)
    std::cout<<"Found:"<<sim_hits.size()<<" sim hits in the "<< hit_collection_<<std::endl;
  
  //Convert to ldmxsps
  for (auto& simHit : sim_hits) {
    
    //Remove low energy deposit hits
    if (simHit.getEdep() >  0.05) {
      
      //Only selects hits that have trackID==1
      if (trackID_ > 0 && simHit.getTrackID() != trackID_)
        continue;
      
      if (pdgID_ != -9999 && abs(simHit.getPdgID()) != pdgID_)
        continue;

      ldmx::LdmxSpacePoint* ldmxsp = utils::convertSimHitToLdmxSpacePoint(simHit);
      
      if (removeStereo_) {
        unsigned int layerid = ldmxsp->layer();
        if (layerid == 3101 || layerid == 3201 || layerid == 3301 || layerid == 3401 )
          continue;
      }

      ldmxsps.push_back(ldmxsp);
    }
    
  }
  
  if (debug_)
    std::cout<<"Hits for fitting:"<<ldmxsps.size()<<std::endl;
    
  //The mapping between the geometry identifier
  //and the IndexSourceLink that points to the hit
  //std::unordered_map<Acts::GeometryIdentifier,
  //                   std::vector< ActsExamples::IndexSourceLink> > geoId_sl_map_;
  std::unordered_multimap<Acts::GeometryIdentifier,
                          ActsExamples::IndexSourceLink> geoId_sl_mmap_;
  
  //Check the hits associated to the surfaces
  for (unsigned int i_ldmx_hit = 0; i_ldmx_hit < ldmxsps.size(); i_ldmx_hit++) {

    ldmx::LdmxSpacePoint* ldmxsp = ldmxsps.at(i_ldmx_hit);
    unsigned int layerid = ldmxsp->layer();

    const Acts::Surface* hit_surface = layer_surface_map_[layerid];
    if (hit_surface) {
      
      //Transform the ldmx space point from global to local and store the information

      
      if (debug_ ) {
        std::cout<<"Global hit position on layer::"<< ldmxsp->layer()<<std::endl;
        std::cout<<ldmxsp->global_pos_<<std::endl;
        hit_surface->toStream(gctx_,std::cout);
        std::cout<<std::endl;
        std::cout<<"TRANSFORM LOCAL TO GLOBAL"<<std::endl;
        std::cout<<hit_surface->transform(gctx_).rotation()<<std::endl;
        std::cout<<hit_surface->transform(gctx_).translation()<<std::endl;
      }
      
      
      Acts::Vector3 dummy_momentum;
      Acts::Vector2 local_pos;
      try { 
        local_pos = hit_surface->globalToLocal(gctx_,ldmxsp->global_pos_,dummy_momentum, 0.320).value();
      } catch (const std::exception& e) {
        std::cout<<"WARNING:: hit not on surface.. Skipping."<<std::endl;
        std::cout<<ldmxsp->global_pos_<<std::endl;
        continue;
      }

      //Smear the local position
      
      if (do_smearing_) {
        float smear_factor{(*normal_)(generator_)};

        if (debug_) {
          std::cout<<"Smearing factor for u="<<smear_factor<<std::endl;
          std::cout<<"Local Pos before::"<<local_pos[0]<<std::endl;
        }
        local_pos[0] += smear_factor * sigma_u_;

        if (debug_)
          std::cout<<"Local Pos after::"<<local_pos[0]<<std::endl;
        
        smear_factor = (*normal_)(generator_);
        if (debug_) {
          std::cout<<"Smearing factor for v="<<smear_factor<<std::endl;
          std::cout<<"Local Pos before::"<<local_pos[1]<<std::endl;
        }
        local_pos[1] += smear_factor * sigma_v_;
        if (debug_)
          std::cout<<"Local Pos after::"<<local_pos[1]<<std::endl;
        
        //update covariance
        ldmxsp->setLocalCovariance(sigma_u_ * sigma_u_, sigma_v_ * sigma_v_);
        
        //update global position
        if (debug_) {
        std::cout<<"Before smearing"<<std::endl;
        std::cout<<ldmxsp->global_pos_<<std::endl;
        }
        
        //cache the acts x coordinate 
        double original_x = ldmxsp->global_pos_(0);

        //transform to global
        ldmxsp->global_pos_ = hit_surface->localToGlobal(gctx_,local_pos,dummy_momentum);

        if (debug_) {
          std::cout<<"The global position after the smearing"<<std::endl;
          std::cout<<ldmxsp->global_pos_<<std::endl;
        }
        
        
        //update the acts x location 
        ldmxsp->global_pos_(0) = original_x;

        //if (debug_) {
        //  std::cout<<"After smearing"<<std::endl;
        //  std::cout<<ldmxsp->global_pos_<<std::endl;
        //}
      }
      
      ldmxsp->local_pos_ = local_pos;

      if (debug_) {
        std::cout<<"Local hit position::"<<std::endl;
        std::cout<<ldmxsp->local_pos_<<std::endl;
      }

      ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(),i_ldmx_hit);

      geoId_sl_mmap_.insert(std::make_pair(hit_surface->geometryId(), idx_sl));
            
    }
    else
      std::cout<<getName()<<"::HIT "<<i_ldmx_hit<<" at layer"<<(ldmxsps.at(i_ldmx_hit))->layer()<<" is not associated to any surface?!"<<std::endl;
    
  }

  auto hits = std::chrono::high_resolution_clock::now();
  profiling_map_["hits"] += std::chrono::duration<double,std::milli>(hits-setup).count();
      
  // ============   Setup the CKF  ============
  Acts::CombinatorialKalmanFilter<Propagator> ckf(*propagator_);  //Acts::Propagagtor<Acts::EigenStepper<>, Acts::Navigator>
  
  //Retrieve the seeds
    
  if (debug_) {
    std::cout<<"Retrieve the seeds::"<< seed_coll_name_<<std::endl;
  }
  
  const std::vector<ldmx::Track> seed_tracks = event.getCollection<ldmx::Track>(seed_coll_name_);

  //Run the CKF on each seed and produce a track candidate
  std::vector<Acts::BoundTrackParameters> startParameters;
  for (auto& seed : seed_tracks) {

    if (debug_) {
      std::cout<<"Seed conversion to track."<<std::endl;
      std::cout<<"perigeeSurface"<<std::endl;
    }
        
    //Transform the seed track to bound parameters
    std::shared_ptr<Acts::PerigeeSurface> perigeeSurface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(seed.getPerigeeX(),
                                                                      seed.getPerigeeY(),
                                                                      seed.getPerigeeZ()));

    
    Acts::BoundVector paramVec;
    paramVec <<
        seed.getD0(),
        seed.getZ0(),
        seed.getPhi(),
        seed.getTheta(),
        seed.getQoP(),
        seed.getT();

    Acts::BoundSymMatrix covMat =
        tracking::sim::utils::unpackCov(seed.getPerigeeCov());
        
    if (debug_) 
      std::cout<<"q"<<std::endl;
    Acts::ActsScalar q = seed.getQoP() < 0 ?
                         -1 * Acts::UnitConstants::e :
                         Acts::UnitConstants::e;
    
    startParameters.push_back(Acts::BoundTrackParameters(perigeeSurface,
                                                         paramVec,
                                                         q,
                                                         covMat));  
  }
  
  if (startParameters.size() != 1 ) {
    std::vector<ldmx::Track> empty;
    event.add(out_trk_collection_,empty);
    return;
  }
   

  nseeds_++ ;
  
  for (auto & startParameter : startParameters) {
    //Already fill the plots with the truth information
    h_p_truth_     ->Fill(startParameters.at(0).absoluteMomentum());
    h_d0_truth_    ->Fill(startParameters.at(0).get<Acts::BoundIndices::eBoundLoc0>());
    h_z0_truth_    ->Fill(startParameters.at(0).get<Acts::BoundIndices::eBoundLoc1>());
    h_phi_truth_   ->Fill(startParameters.at(0).get<Acts::BoundIndices::eBoundPhi>());
    h_theta_truth_ ->Fill(startParameters.at(0).get<Acts::BoundIndices::eBoundTheta>());
    h_qop_truth_   ->Fill(startParameters.at(0).get<Acts::BoundIndices::eBoundQOverP>());
  }

  auto seeds = std::chrono::high_resolution_clock::now();
  profiling_map_["seeds"] += std::chrono::duration<double,std::milli>(seeds-hits).count();
  
  Acts::GainMatrixUpdater kfUpdater;
  Acts::GainMatrixSmoother kfSmoother;

  // configuration for the measurement selector. Empty geometry identifier means applicable to all the detector
  // elements

  Acts::MeasurementSelector::Config measurementSelectorCfg = {
    // global default: no chi2 cut, only one measurement per surface                                                                                                                                                    
    {Acts::GeometryIdentifier(),
     {{}, {std::numeric_limits<double>::max()}, {1u}}},
  };
  
  Acts::MeasurementSelector measSel{measurementSelectorCfg};
  LdmxMeasurementCalibrator calibrator{ldmxsps};
  
  Acts::CombinatorialKalmanFilterExtensions ckf_extensions;

  if (use1Dmeasurements_)
    ckf_extensions.calibrator.connect<&LdmxMeasurementCalibrator::calibrate_1d>(&calibrator);
  else 
    ckf_extensions.calibrator.connect<&LdmxMeasurementCalibrator::calibrate>(&calibrator);
  ckf_extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(&kfUpdater);
  ckf_extensions.smoother.connect<&Acts::GainMatrixSmoother::operator()>(&kfSmoother);
  ckf_extensions.measurementSelector.connect<&Acts::MeasurementSelector::select>(&measSel);
    
  using LdmxSourceLinkAccessor = GeneralContainerAccessor<std::unordered_multimap<Acts::GeometryIdentifier, ActsExamples::IndexSourceLink> >  ;
  
  //not supported anymore
  //auto extr_surface = &(*gen_surface);

  std::shared_ptr<const Acts::PerigeeSurface> origin_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(0., 0., 0.));
  
  auto extr_surface = &(*origin_surface);
      
  std::shared_ptr<const Acts::PerigeeSurface> tgt_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(extrapolate_location_[0], extrapolate_location_[1], extrapolate_location_[2]));
  
  if (use_extrapolate_location_) {
    extr_surface = &(*tgt_surface);
  }
  
  Acts::Vector3 seed_perigee_surface_center = startParameters.at(0).referenceSurface().center(gctx_);
  std::shared_ptr<const Acts::PerigeeSurface> seed_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(seed_perigee_surface_center);
  

  if (use_seed_perigee_) {
    
    extr_surface = &(*seed_surface);
  }

  auto ckf_loggingLevel = Acts::Logging::FATAL;
  if (debug_)
    ckf_loggingLevel = Acts::Logging::VERBOSE;
  const auto ckflogger = Acts::getDefaultLogger("CKF", ckf_loggingLevel);
  Acts::CombinatorialKalmanFilterOptions<LdmxSourceLinkAccessor> kfOptions(
      gctx_,bctx_,cctx_,
      LdmxSourceLinkAccessor(), ckf_extensions, Acts::LoggerWrapper{*ckflogger},
      propagator_options,&(*extr_surface));
  
  // run the CKF for all initial track states

  auto ckf_setup = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_setup"] += std::chrono::duration<double,std::milli>(ckf_setup-seeds).count();
  
  auto results = ckf.findTracks(geoId_sl_mmap_, startParameters, kfOptions);

  auto ckf_run = std::chrono::high_resolution_clock::now();
  profiling_map_["ckf_run"] += std::chrono::duration<double,std::milli>(ckf_run - ckf_setup).count();

  //std::cout<<"StartParameters size::"<<startParameters.size()<<std::endl;
  //std::cout<<"results size::"<<results.size()<<std::endl;

  int ResultIndex = 0;
  int GoodResult = 0;
  
  for (auto& result : results) {

    ResultIndex ++ ;
    if (!result.ok()) {
      continue;
    }
    GoodResult++;
    
    auto ckf_result = result.value();

    
    if (debug_) {
      std::cout<<"Track Index::"<<GoodResult - 1<<std::endl;
      std::cout<<"==============="<<std::endl;
      std::cout<<"ckf_result:: filtered " << ckf_result.filtered<<std::endl;
      std::cout<<"ckf_result:: smoothed " << ckf_result.smoothed<<std::endl;
      std::cout<<"ckf_result:: iSmoothed "<< ckf_result.iSmoothed<<std::endl;
      std::cout<<"ckf_result:: finished " << ckf_result.finished<<std::endl;
      std::cout<<"Size of the active Tips "<< ckf_result.activeTips.size()<<std::endl;
      std::cout<<"Last Measurement indices " << ckf_result.lastMeasurementIndices.size()<<std::endl;
      for (auto & lm : ckf_result.lastMeasurementIndices) {
        std::cout<<"LastMeasurementIndex="<<lm<<std::endl;
      }
      std::cout<<"Last Track indices " << ckf_result.lastTrackIndices.size()<<std::endl;
      for (auto & lt : ckf_result.lastTrackIndices) {
        std::cout<<"LastTrackIndex="<<lt<<std::endl;
      }
    }
    
    //The track tips are the last measurement index
    Acts::MultiTrajectory mj = ckf_result.fittedStates;
    
    //In the current case I should have a single trackTip
    //Check https://github.com/acts-project/acts/blob/8f1f47bb57044b3e476d01b3dbafb13030038bd5/Examples/Io/Performance/ActsExamples/Io/Performance/TrackFitterPerformanceWriter.cpp#L114
    auto trackTip = ckf_result.lastMeasurementIndices.front();
    
    // Collect the trajectory summary info
    auto trajState =
        Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
    
    //Check some track info
    
    if (debug_) {
      std::cout<<"nStates="<<trajState.nStates<<std::endl;
      std::cout<<"nMeasurements="<<trajState.nMeasurements<<std::endl;
      std::cout<<"nOutliers="<<trajState.nOutliers<<std::endl;
      std::cout<<"nHoles="<<trajState.nHoles<<std::endl;
      std::cout<<"chi2sum="<<trajState.chi2Sum<<std::endl;
      std::cout<<"NDF="<<trajState.NDF<<std::endl;
      std::cout<<"nsharedHits="<<trajState.nSharedHits<<std::endl;
      std::cout<<"###########"<<std::endl;
    }

    if (trajState.nStates != trajState.nMeasurements + trajState.nOutliers + trajState.nHoles) {
      if (debug_)
        std::cout<<"WARNING:: Found track with nStates inconsistent with expectation"<<std::endl;
      //continue;
    }

    //Cut on number of hits?
    if (trajState.nMeasurements < minHits_)
      continue;

    h_nHits_->Fill(trajState.nMeasurements);
        
    for (const auto& pair : ckf_result.fittedParameters) {
      //std::cout<<"Number of hits-on-track::" << (int) pair.first << std::endl;


      double resp     = pair.second.absoluteMomentum() - startParameters.at(0).absoluteMomentum();
      double resd0    = pair.second.get<Acts::BoundIndices::eBoundLoc0>() - startParameters.at(0).get<Acts::BoundIndices::eBoundLoc0>();
      double resz0    = pair.second.get<Acts::BoundIndices::eBoundLoc1>() - startParameters.at(0).get<Acts::BoundIndices::eBoundLoc1>();
      double resphi   = pair.second.get<Acts::BoundIndices::eBoundPhi>() - startParameters.at(0).get<Acts::BoundIndices::eBoundPhi>();
      double restheta = pair.second.get<Acts::BoundIndices::eBoundTheta>() - startParameters.at(0).get<Acts::BoundIndices::eBoundTheta>();
      histo_p_    ->Fill(resp);
      histo_d0_   ->Fill(resd0);
      histo_z0_   ->Fill(resz0);
      histo_phi_  ->Fill(resphi);
      histo_theta_->Fill(restheta);
      
      h_p_    ->Fill(pair.second.absoluteMomentum());
      h_d0_   ->Fill(pair.second.get<Acts::BoundIndices::eBoundLoc0>());
      h_z0_   ->Fill(pair.second.get<Acts::BoundIndices::eBoundLoc1>());
      h_phi_  ->Fill(pair.second.get<Acts::BoundIndices::eBoundPhi>());
      h_theta_->Fill(pair.second.get<Acts::BoundIndices::eBoundTheta>());
      h_qop_  ->Fill(pair.second.get<Acts::BoundIndices::eBoundQOverP>());
      
      //auto cov = pair.second.covariance();
      //auto stddev = cov.diagonal().cwiseSqrt().eval();
      
      auto cov = pair.second.covariance();
      double sigma_d0    = sqrt(cov.value()(Acts::BoundIndices::eBoundLoc0,Acts::BoundIndices::eBoundLoc0));
      double sigma_z0    = sqrt(cov.value()(Acts::BoundIndices::eBoundLoc1,Acts::BoundIndices::eBoundLoc1));
      double sigma_phi   = sqrt(cov.value()(Acts::BoundIndices::eBoundPhi,Acts::BoundIndices::eBoundPhi));
      double sigma_theta = sqrt(cov.value()(Acts::BoundIndices::eBoundTheta,Acts::BoundIndices::eBoundTheta));
      double sigma_qop   = sqrt(cov.value()(Acts::BoundIndices::eBoundQOverP,Acts::BoundIndices::eBoundQOverP));
                  
      h_d0_err_   ->Fill(sigma_d0);
      h_z0_err_   ->Fill(sigma_z0);  
      h_phi_err_  ->Fill(sigma_phi);  
      h_theta_err_->Fill(sigma_theta);  
      h_qop_err_  ->Fill(sigma_qop);

      
      histo_d0_pull_    ->Fill(resd0/sigma_d0);
      histo_z0_pull_    ->Fill(resz0/sigma_z0);
      histo_phi_pull_   ->Fill(resphi/sigma_phi);
      histo_theta_pull_ ->Fill(restheta/sigma_theta);
      
      double sigma_p = pair.second.absoluteMomentum() * pair.second.absoluteMomentum() * sigma_qop;
      
      h_p_err_         ->Fill(sigma_p);
      histo_p_pull_    ->Fill(resp / sigma_p);
            
    }

    //Create a track object

    ldmx::Track trk = ldmx::Track();
    trk.setPerigeeLocation(extr_surface->center(gctx_)[0],
                           extr_surface->center(gctx_)[1],
                           extr_surface->center(gctx_)[2]);
    
    trk.setChi2(trajState.chi2Sum);
    trk.setNhits(trajState.nMeasurements);
    trk.setNdf(trajState.NDF);
    trk.setNsharedHits(trajState.nSharedHits);
    
    //Set the perigee parameters (TODO:: There should only be one single track per finding-fit for now. This method needs to be updated)
    //trk.setPerigeeParameters(ckf_result.fittedParameters.begin()->second.parameters());

    Acts::BoundVector tgt_srf_pars = ckf_result.fittedParameters.begin()->second.parameters();
    Acts::BoundMatrix tgt_srf_cov  = ckf_result.fittedParameters.begin()->second.covariance() ?
                                     ckf_result.fittedParameters.begin()->second.covariance().value() : Acts::BoundSymMatrix::Identity();

    /*
    if (ckf_result.fittedParameters.begin()->second.charge() > 0) {
      std::cout<<getName()<<"::ERROR!!! ERROR!! Found track with q>0. Chi2="<<trajState.chi2Sum<<std::endl;
      mj.visitBackwards(trackTip, [&](const auto& state) {
        std::cout<<"Printing smoothed states"<<std::endl;
        std::cout<<state.smoothed()<<std::endl;
        std::cout<<"Printing filtered states"<<std::endl;
        std::cout<<state.filtered()<<std::endl;
      });
      
      
    }*/
    
    
    if (debug_) {
      std::cout<<"Check parameters:"<<std::endl;
      std::cout<<tgt_srf_pars<<std::endl;
      
      std::cout<<"Check covariance:"<<std::endl;
      std::cout<<tgt_srf_cov<<std::endl;
    }
    
    
    std::vector<double> v_tgt_srf_pars(tgt_srf_pars.data(), tgt_srf_pars.data() + tgt_srf_pars.rows() * tgt_srf_pars.cols());
    std::vector<double> v_tgt_srf_cov_flat;
    tracking::sim::utils::flatCov(tgt_srf_cov, v_tgt_srf_cov_flat);
    Acts::Vector3 trk_momentum = ckf_result.fittedParameters.begin()->second.momentum();
    Acts::Vector3 trk_position = ckf_result.fittedParameters.begin()->second.position(gctx_);
    
    
    if (debug_) {
      for (auto& p : v_tgt_srf_pars) {
        std::cout<<"par:"<<p<<std::endl;
      }
      
      for (auto& c : v_tgt_srf_cov_flat) {
        std::cout<<"cov flat:"<<c<<std::endl;
      }
    }

    if (debug_) {
      std::cout<<"Filling the track Perigee par/cov"<<std::endl;
    }
    
    trk.setPerigeeParameters(v_tgt_srf_pars);
    trk.setPerigeeCov(v_tgt_srf_cov_flat);
    trk.setMomentum(trk_momentum(0), trk_momentum(1), trk_momentum(2));
    trk.setPosition(trk_position(0), trk_position(1), trk_position(2));

    
    tracks.push_back(trk);
    ntracks_++;

    if (ckf_result.fittedParameters.begin()->second.absoluteMomentum() < 1.2 && false)
      //Write the event display for the recoil
      WriteEvent(event,
		 ckf_result.fittedParameters.begin()->second,
                 mj,
		 trackTip,
                 ldmxsps);

    
    //Refit the track with the KalmanFitter using backward propagation

    bool kfRefit = false;
    if (kfRefit) {

      std::cout<<"Preparing theKF refit"<<std::endl;
      std::vector<std::reference_wrapper<const ActsExamples::IndexSourceLink>> fit_trackSourceLinks;
      mj.visitBackwards(trackTip, [&](const auto& state) {
        const auto& sourceLink =
          static_cast<const ActsExamples::IndexSourceLink&>(state.uncalibrated());
        auto typeFlags = state.typeFlags();
        if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
          fit_trackSourceLinks.push_back(std::cref(sourceLink));
        }
      });

      std::cout<<"Getting the logger and adding the extensions."<<std::endl;
      const auto kfLogger = Acts::getDefaultLogger("KalmanFitter", Acts::Logging::INFO);
      Acts::KalmanFitterExtensions kfitter_extensions;
      kfitter_extensions.calibrator.connect<&LdmxMeasurementCalibrator::calibrate_1d>(&calibrator);
      kfitter_extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(&kfUpdater);
      kfitter_extensions.smoother.connect<&Acts::GainMatrixSmoother::operator()>(&kfSmoother);

      //Rewrite all the stuff from scratch
      


      //rFiltering is true, so it should run in reversed direction.
      
      Acts::KalmanFitterOptions kfitter_options =
          Acts::KalmanFitterOptions(gctx_,bctx_,cctx_,
                                    kfitter_extensions,Acts::LoggerWrapper{*kfLogger},
                                    propagator_options,&(*extr_surface),
                                    true, true, true); //mScattering, exoLoss, rFiltering
      


      std::cout<<"rFiltering =" << (int)kfitter_options.reversedFiltering <<std::endl;
      
      // create the Kalman Fitter
      if (debug_)
        std::cout<<"Make the KalmanFilter fitter object"<<std::endl;
      Acts::KalmanFitter<Propagator> kf(*propagator_);
      
      if (debug_)
        std::cout<<"Refit"<<std::endl;

      std::cout<<"Refit tracks with KF"<<std::endl;
      std::cout<<"Starting from "<<std::endl;
      std::cout<<ckf_result.fittedParameters.begin()->second.position(gctx_)<<std::endl;
      std::cout<<"With momenutm"<<std::endl;
      std::cout<<ckf_result.fittedParameters.begin()->second.momentum()<<std::endl;
      std::cout<<"And field" <<std::endl;
      
      //Acts::MagneticFieldProvider::Cache cache = sp_interpolated_bField_->makeCache(bctx_);
      //std::cout<<" BField::\n"<<sp_interpolated_bField_->getField(ckf_result.fittedParameters.begin()->second.position(gctx_),cache).value() / Acts::UnitConstants::T <<std::endl; 
      
      auto kf_refit_result = kf.fit(fit_trackSourceLinks.begin(),fit_trackSourceLinks.end(),
                                    ckf_result.fittedParameters.begin()->second,kfitter_options);
     
      if (!kf_refit_result.ok()) {
        std::cout<<"KF Refit failed"<<std::endl;
      }
      else {
        
        auto kf_refit_value = kf_refit_result.value();
        auto kf_params = kf_refit_value.fittedParameters;
        h_p_refit_     ->Fill((*kf_params).absoluteMomentum());
        h_d0_refit_    ->Fill((*kf_params).get<Acts::BoundIndices::eBoundLoc0>());
        h_z0_refit_    ->Fill((*kf_params).get<Acts::BoundIndices::eBoundLoc1>());
        h_phi_refit_   ->Fill((*kf_params).get<Acts::BoundIndices::eBoundPhi>());
        h_theta_refit_ ->Fill((*kf_params).get<Acts::BoundIndices::eBoundTheta>());
      }
      
    }//Run the refit


    //Refit track using the GSF

    bool gsfRefit = false;
    
    if (gsfRefit) {

      try {

        
        const auto gsfLogger = Acts::getDefaultLogger("GSF",Acts::Logging::INFO);
        std::vector<std::reference_wrapper<const ActsExamples::IndexSourceLink>> fit_trackSourceLinks;
        mj.visitBackwards(trackTip, [&](const auto& state) {
          const auto& sourceLink =
              static_cast<const ActsExamples::IndexSourceLink&>(state.uncalibrated());
          auto typeFlags = state.typeFlags();
          if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
            fit_trackSourceLinks.push_back(std::cref(sourceLink));
          }
        });

        
        //Same extensions of the KF
        Acts::KalmanFitterExtensions gsf_extensions;
        gsf_extensions.calibrator.connect<&LdmxMeasurementCalibrator::calibrate_1d>(&calibrator);
        gsf_extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(&kfUpdater);
        gsf_extensions.smoother.connect<&Acts::GainMatrixSmoother::operator()>(&kfSmoother);


        
        Acts::GsfOptions gsf_options{gctx_,
          bctx_,
          cctx_,
          gsf_extensions,
          Acts::LoggerWrapper{*gsfLogger},
          propagator_options,
          &(*extr_surface),
          true,
          4,
          false};
        
        
        
        
        auto gsf_refit_result = gsf_->fit(fit_trackSourceLinks.begin(),
                                          fit_trackSourceLinks.end(),
                                          ckf_result.fittedParameters.begin()->second,
                                          gsf_options);
        
        
        if (!gsf_refit_result.ok()) {
          std::cout<<"GSF Refit failed"<<std::endl;
        }
        else {
          
          auto gsf_refit_value = gsf_refit_result.value();
          auto gsf_params = gsf_refit_value.fittedParameters;
          h_p_gsf_refit_     ->Fill((*gsf_params).absoluteMomentum());
          h_d0_gsf_refit_    ->Fill((*gsf_params).get<Acts::BoundIndices::eBoundLoc0>());
          h_z0_gsf_refit_    ->Fill((*gsf_params).get<Acts::BoundIndices::eBoundLoc1>());
          h_phi_gsf_refit_   ->Fill((*gsf_params).get<Acts::BoundIndices::eBoundPhi>());
          h_theta_gsf_refit_ ->Fill((*gsf_params).get<Acts::BoundIndices::eBoundTheta>());
        }
        
      } catch (...) {

        std::cout<<"ERROR:: GSF Refit failed"<<std::endl;
      }
        
      } // do refit GSF
      
  } // loop on CKF Results


  auto result_loop = std::chrono::high_resolution_clock::now();
  profiling_map_["result_loop"] += std::chrono::duration<double,std::milli>(result_loop - ckf_run).count();
  
  if (debug_) 
    std::cout<<"Found "<<GoodResult<< " tracks" <<std::endl;  
  
  
  //Add the tracks to the event
  event.add(out_trk_collection_, tracks);
  
  auto end = std::chrono::high_resolution_clock::now();
  //long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  auto diff = end-start;
  processing_time_ += std::chrono::duration <double, std::milli> (diff).count();
    
}


void TrackingGeometryMaker::onProcessEnd() {
  
  
  std::cout<<"Producer " << getName() << " found " << ntracks_ <<" tracks  / " << nseeds_ << " nseeds"<<std::endl;
  
  
  TFile* outfile_ = new TFile((getName()+".root").c_str(),"RECREATE");
  outfile_->cd();

  histo_p_->Write();
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
  //h_nHits_gsf_refit_->Write();

  h_p_truth_->Write();
  h_d0_truth_->Write();
  h_z0_truth_->Write();
  h_phi_truth_->Write();
  h_theta_truth_->Write();
  h_qop_truth_->Write();
  
  outfile_->Close();  
  delete outfile_;

  std::cout<<"PROCESSOR:: "<<this->getName()<<"   AVG Time/Event: " <<processing_time_ / nevents_ << " ms"<<std::endl;

  std::cout<<"Breakdown::"<<std::endl;
  std::cout<<"setup       Avg Time/Event = "<<profiling_map_["setup"]       / nevents_ << " ms"<<std::endl;
  std::cout<<"hits        Avg Time/Event = "<<profiling_map_["hits"]        / nevents_ << " ms"<<std::endl;
  std::cout<<"seeds       Avg Time/Event = "<<profiling_map_["seeds"]       / nevents_ << " ms"<<std::endl;
  std::cout<<"cf_setup    Avg Time/Event = "<<profiling_map_["ckf_setup"]   / nevents_ << " ms"<<std::endl;
  std::cout<<"ckf_run     Avg Time/Event = "<<profiling_map_["ckf_run"]     / nevents_ << " ms"<<std::endl;
  std::cout<<"result_loop Avg Time/Event = "<<profiling_map_["result_loop"] / nevents_ << " ms"<<std::endl;
  
}


Acts::CuboidVolumeBuilder::VolumeConfig TrackingGeometryMaker::volumeBuilder_gdml(Acts::Logging::Level logLevel) {

  std::vector<std::shared_ptr<const Acts::Surface>> surfaces;

  Acts::CuboidVolumeBuilder::VolumeConfig test;
  
  return test;
}

//A copy is not a good idea. TODO
Acts::CuboidVolumeBuilder::VolumeConfig TrackingGeometryMaker::volumeBuilder_dd4hep(dd4hep::DetElement& subdetector,Acts::Logging::Level logLevel) {
  
  // Get the extension, if it exists
  Acts::ActsExtension* subDetExtension = nullptr;
  try {
    subDetExtension = subdetector.extension<Acts::ActsExtension>();
  } catch (std::runtime_error& e) {
  }
    
  //Just a place holder in the case we will make compound detectors. 
  if (subdetector.type() == "compound") {}
  // Now create the Layerbuilders and Volumebuilder

  // Define the configuration for the cuboid volume builder object 
  Acts::CuboidVolumeBuilder::Config cvbConfig;
    
  // Get the sensors
  std::vector<dd4hep::DetElement> sensors;
    
  //collect the sensors (just a trick to add the ActsExtension)
    
  collectSensors_dd4hep(subdetector,sensors);
  if (debug_)
    std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" the size of sensors="<<sensors.size()<<std::endl;
    
  std::vector<std::shared_ptr<const Acts::Surface>> surfaces;
    
  //Get all the sensitive surfaces for the tagger Tracker. 
  //For the moment I'm forcing to grep everything.
  
  //resolveSensitive(subdetector,surfaces,true);
  //if (debug_)
  //  std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" surfaces size::"<<surfaces.size()<<std::endl;
    
  //Check the surfaces that I created (but I will not use)

  //if (debug_)  {
    
  //  for (auto& surface : surfaces) {
      
  //    surface->toStream(gctx_,std::cout);
  //    std::cout<<std::endl;
  //    surface->surfaceMaterial()->toStream(std::cout);
  //    std::cout<<std::endl;
  //    std::cout<<surface->surfaceMaterial()->materialSlab(0,0).material().massDensity()<<std::endl;
  //    std::cout<<surface->surfaceMaterial()->materialSlab(0,0).material().molarDensity()<<std::endl;
  //  }
  //}
    
  //Surfaces configs
  //This bypasses the surfaces built before
  std::vector<Acts::CuboidVolumeBuilder::SurfaceConfig> surfaceConfig;
  
  //Reorder the sensors in ascending order along the beam axis.
  //TODO::USE A MAP instead of all these vector manipulations//
    
  std::sort(sensors.begin(),sensors.end(),[](const dd4hep::DetElement& lhs,
                                             const dd4hep::DetElement& rhs) {
    const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
    const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
    return t_lhs[2] <= t_rhs[2];
  });
  
  //Check if I'm doing the recoil
  if (sensors.size() > 25 ) {
    
    //Now all the recoil sensors should be at the end of the sensor vector (10 elements).
    //Let's sort the first 5 and then the second 5

    // - First sort by y
    
    std::sort(sensors.end() - 20, sensors.end() - 10,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[1] > t_rhs[1];
              });
    
    // - Then sort by x
    
    std::sort(sensors.end() - 20, sensors.end() - 15,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });

    std::sort(sensors.end() - 15, sensors.end() - 10,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });

    
    // - First sort by y
    
    std::sort(sensors.end() - 10, sensors.end(),
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[1] > t_rhs[1];
              });
    
    // - Then sort by x
    
    std::sort(sensors.end() - 10, sensors.end() - 5,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });

    std::sort(sensors.end() - 5, sensors.end(),
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });
    
  } // recoil check
  
  //Prepare the surface configurations 
  int counter = 0;
  for (auto& sensor : sensors) {
    counter++;
    
    Acts::CuboidVolumeBuilder::SurfaceConfig cfg;

    if (debug_)
      std::cout<<"Getting the transform "<<sensor.name()<<std::endl;
        
    //Get the tranformation from the alignment support. 
    auto transform =
        convertTransform(&(sensor.nominal().worldTransformation()));

    //Rotate Z->X, X->Y, Y->Z 
    Acts::Vector3 position = {transform.translation()[2], transform.translation()[0], transform.translation()[1]};
        
    //Rotate the sensors to be orthogonal to X
    double rotationAngle = M_PI * 0.5;
        
    // 0 0 -1
    // 0 1 0
    // 1 0 0
        
    //This rotation is needed to have the plane orthogonal to the X direction. 
    // Rotation of the surfaces
    Acts::Vector3 xPos1(cos(rotationAngle), 0., sin(rotationAngle));
    Acts::Vector3 yPos1(0., 1., 0.);
    Acts::Vector3 zPos1(-sin(rotationAngle), 0., cos(rotationAngle));
        
    Acts::RotationMatrix3 y_rot;
    y_rot.col(0) = xPos1;
    y_rot.col(1) = yPos1;
    y_rot.col(2) = zPos1;

    //Rotate the sensors to put them in the proper orientation in Z
    Acts::Vector3 xPos2(1., 0. ,0.);
    Acts::Vector3 yPos2(0., cos(rotationAngle), sin(rotationAngle));
    Acts::Vector3 zPos2(0., -sin(rotationAngle),cos(rotationAngle));

    Acts::RotationMatrix3 x_rot;
    x_rot.col(0) = xPos2;
    x_rot.col(1) = yPos2;
    x_rot.col(2) = zPos2;
        
        
    cfg.position = position;
    //cfg.rotation = cfg.rotation*transform.rotation();
    cfg.rotation = x_rot*y_rot*transform.rotation();

    /*
    if (debug_) {
      std::cout<<"POSITION AND ROTATION OF THE SURFACES"<<std::endl;
      //Position and rotation of the surface
      std::cout<<cfg.position<<std::endl;
      std::cout<<cfg.rotation<<std::endl;
    }
    */
    
    //Get the bounds - 
    //cfg.rBounds  = std::make_shared<const Acts::RectangleBounds>(Acts::RectangleBounds(20.17, 50));
    
    cfg.rBounds  = std::make_shared<const Acts::RectangleBounds>(
        Acts::RectangleBounds(Acts::UnitConstants::cm*sensor.volume().boundingBox().x(),
                              Acts::UnitConstants::cm*sensor.volume().boundingBox().y()));
    
    // I don't think I need this to be defined.
    cfg.detElementConstructor = nullptr; 
        
    // Thickness. The units in dd4hep are in cm, that's why scaling. And they are in half lengths, that's why x2
    double thickness = 2*Acts::UnitConstants::cm*sensor.volume().boundingBox().z();
                    
    // Material
        
    dd4hep::Material de_mat = sensor.volume().material();

    /*
    Acts::Material silicon = Acts::Material::fromMassDensity(de_mat.radLength() * Acts::UnitConstants::mm,
                                                             de_mat.intLength() * Acts::UnitConstants::mm,
                                                             de_mat.A(),
                                                             de_mat.Z(),
                                                             de_mat.density() * Acts::UnitConstants::g / Acts::UnitConstants::cm3);
    */
    Acts::Material silicon = Acts::Material::fromMassDensity(95.7 * Acts::UnitConstants::mm,
                                                             465.2 * Acts::UnitConstants::mm,
                                                             28.03,
                                                             14.,
                                                             2.32 * Acts::UnitConstants::g / Acts::UnitConstants::cm3);
    
    Acts::MaterialSlab silicon_slab(silicon,thickness * Acts::UnitConstants::mm); 
    cfg.thickness = thickness;
    cfg.surMat = std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab);


    //Store the configurations
    std::string sensor_name=sensor.name();

    if (sensor_name == "tagger_tracker_layer_1" || sensor_name == "tagger_tracker_layer_2") {
      tracker_layout["tagger_tracker_layer_L1"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_3" || sensor_name == "tagger_tracker_layer_4") {
      tracker_layout["tagger_tracker_layer_L2"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_5" || sensor_name == "tagger_tracker_layer_6") {
      tracker_layout["tagger_tracker_layer_L3"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_7" || sensor_name == "tagger_tracker_layer_8") {
      tracker_layout["tagger_tracker_layer_L4"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_9" || sensor_name == "tagger_tracker_layer_10") {
      tracker_layout["tagger_tracker_layer_L5"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_11" || sensor_name == "tagger_tracker_layer_12") {
      tracker_layout["tagger_tracker_layer_L6"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_13" || sensor_name == "tagger_tracker_layer_14") {
      tracker_layout["tagger_tracker_layer_L7"].push_back(cfg);
    }
    
    /* recoil association */
    
    if (sensor_name == "recoil_tracker_layer_1" || sensor_name == "recoil_tracker_layer_2") {
      tracker_layout["recoil_tracker_layer_L1"].push_back(cfg);
    }
    
    if (sensor_name == "recoil_tracker_layer_3" || sensor_name == "recoil_tracker_layer_4") {
      tracker_layout["recoil_tracker_layer_L2"].push_back(cfg);
    }
        
    if (sensor_name == "recoil_tracker_layer_5" || sensor_name == "recoil_tracker_layer_6") {
      tracker_layout["recoil_tracker_layer_L3"].push_back(cfg);
    }

    if (sensor_name == "recoil_tracker_layer_7" || sensor_name == "recoil_tracker_layer_8") {
      tracker_layout["recoil_tracker_layer_L4"].push_back(cfg);
    }
    
    if (sensor_name == "recoil_tracker_layer_9" ||
        sensor_name == "recoil_tracker_layer_10" ||
        sensor_name == "recoil_tracker_layer_11" ||
        sensor_name == "recoil_tracker_layer_12" ||
        sensor_name == "recoil_tracker_layer_13" ||
        sensor_name == "recoil_tracker_layer_14" ||
        sensor_name == "recoil_tracker_layer_15" ||
        sensor_name == "recoil_tracker_layer_16" ||
        sensor_name == "recoil_tracker_layer_17" ||
        sensor_name == "recoil_tracker_layer_18" 
        ) {
      tracker_layout["recoil_tracker_layer_L5"].push_back(cfg);
    }

    if (sensor_name == "recoil_tracker_layer_19" ||
        sensor_name == "recoil_tracker_layer_20" ||
        sensor_name == "recoil_tracker_layer_21" ||
        sensor_name == "recoil_tracker_layer_22" ||
        sensor_name == "recoil_tracker_layer_23" ||
        sensor_name == "recoil_tracker_layer_24" ||
        sensor_name == "recoil_tracker_layer_25" ||
        sensor_name == "recoil_tracker_layer_26" ||
        sensor_name == "recoil_tracker_layer_27" ||
        sensor_name == "recoil_tracker_layer_28" 
        ) {
      tracker_layout["recoil_tracker_layer_L6"].push_back(cfg);
    }
        
  } // sensors loop
  
  //if (debug_)
  //  std::cout<<"Formed " <<surfaceConfig.size()<< " Surface configs"<<std::endl;
  
  //Layer Configurations
  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layerConfig;
    
  for (auto const& x : tracker_layout)
  {
    /*
    if (debug_) {
      std::cout << x.first  
                << ": surfaces==>" 
                << x.second.size()
                << std::endl;
    }
    */
    
    Acts::CuboidVolumeBuilder::LayerConfig lcfg;
    lcfg.surfaceCfg = x.second;
    //lcfg.rotation = Acts::RotationMatrix3::Identity();
    double clearance = 0.01; //0.001
    lcfg.envelopeX = std::pair<double,double>{x.second.front().thickness / 2. + clearance, x.second.front().thickness / 2. + clearance};
    layerConfig.push_back(lcfg);
    lcfg.active = true;
  }
  
  
  
  if (debug_)
    std::cout<<"Formed " <<layerConfig.size()<< " layer configs"<<std::endl;
    
  //Create the volume

  if (debug_)
    std::cout<<"FORMING the boundaries for:"<<subdetector.name()<<std::endl;
  // Build the sub-detector volume configuration
  Acts::CuboidVolumeBuilder::VolumeConfig subDetVolumeConfig;
    
  // Get the transform wrt the world
  auto subDet_transform = convertTransform(&(subdetector.nominal().worldTransformation()));

  if (debug_) {
    std::cout<<subDet_transform.translation()<<std::endl;
    std::cout<<subDet_transform.rotation()<<std::endl;
  }

  //Rotate..Z->X, X->Y, Y->Z
  //Add 1mm to not make it sit on the first layer surface
  Acts::Vector3 sub_det_position = {subDet_transform.translation()[2]-1, subDet_transform.translation()[0], subDet_transform.translation()[1]};
  
  double x_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().z()+1;
  double y_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().x();
  double z_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().y();

  //Larger volume to check propagation in the recoil area
  //x_length = 7*Acts::UnitConstants::cm*subdetector.volume().boundingBox().z()+1;
  
  if (debug_)
    std::cout<<"x "<<x_length<<" y "<<y_length<<" z "<<z_length<<std::endl;

  subDetVolumeConfig.position = sub_det_position;
  subDetVolumeConfig.length = {x_length, y_length, z_length};
  subDetVolumeConfig.layerCfg = layerConfig;
  subDetVolumeConfig.name = subdetector.name();
    
  //Form the Homogeneous material for the tagger volume
  dd4hep::Material subde_mat = subdetector.volume().material();
  Acts::Material subdet_mat = Acts::Material::fromMassDensity(subde_mat.radLength(),
                                                              subde_mat.intLength(), subde_mat.A(), 
                                                              subde_mat.Z(), subde_mat.density()); 
  
  subDetVolumeConfig.volumeMaterial =
      std::make_shared<Acts::HomogeneousVolumeMaterial>(subdet_mat);
  

  //Clear up the layout map to accept the new sub-detector
  tracker_layout.clear();
  
  return subDetVolumeConfig;
    
}

  
void TrackingGeometryMaker::configure(framework::config::Parameters &parameters) {
    
  dumpobj_            = parameters.getParameter<int>("dumpobj");
  steps_outfile_path_ = parameters.getParameter<std::string>("steps_file_path","propagation_steps.root");
  trackID_            = parameters.getParameter<int>("trackID",-1);
  pdgID_              = parameters.getParameter<int>("pdgID",11);
  
  bfield_               = parameters.getParameter<double>("bfield", 0.);
  const_b_field_        = parameters.getParameter<bool>("const_b_field",true);
  bfieldMap_            = parameters.getParameter<std::string>("bfieldMap_",
                                                               "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat");
  propagator_step_size_ = parameters.getParameter<double>("propagator_step_size", 200.);
  propagator_maxSteps_  = parameters.getParameter<int>("propagator_maxSteps", 10000);
  perigee_location_     = parameters.getParameter<std::vector<double> >("perigee_location", {0.,0.,0.});
  debug_                = parameters.getParameter<bool>("debug",false);
  hit_collection_       = parameters.getParameter<std::string>("hit_collection","TaggerSimHits");

  removeStereo_         = parameters.getParameter<bool>("removeStereo",false);
  if (removeStereo_)
    std::cout<<"CONFIGURE::removeStereo="<<(int)removeStereo_<<std::endl;

  use1Dmeasurements_    = parameters.getParameter<bool>("use1Dmeasurements",true);
  
  if (use1Dmeasurements_)
    std::cout<<"CONFIGURE::use1Dmeasurements="<<(int)use1Dmeasurements_<<std::endl;

  minHits_              = parameters.getParameter<int>("minHits",7);

  std::cout<<"CONFIGURE::minHits="<<minHits_<<std::endl;
                                                        
  //Ckf specific options
  use_extrapolate_location_  = parameters.getParameter<bool>("use_extrapolate_location", true);
  extrapolate_location_ = parameters.getParameter<std::vector<double> >("extrapolate_location", {0.,0.,0.});
  use_seed_perigee_ = parameters.getParameter<bool>("use_seed_perigee", false);
  
  //seeds from the event
  seed_coll_name_     = parameters.getParameter<std::string>("seed_coll_name","seedTracks");
  
  //output track collection
  out_trk_collection_ = parameters.getParameter<std::string>("out_trk_collection", "Tracks");
  
  
  //Hit smearing

  do_smearing_ = parameters.getParameter<bool>("do_smearing",false);
  sigma_u_ = parameters.getParameter<double>("sigma_u", 0.01);
  sigma_v_ = parameters.getParameter<double>("sigma_v", 0.);
    
  std::cout<<__PRETTY_FUNCTION__<<"  HitCollection::"<<hit_collection_<<std::endl;
}

// **** //
//std::shared_ptr<const Acts::CylinderVolumeHelper> cuboidVolumeHelper_dd4hep(Logging::Level loggingLevel) {
//#####################//

void TrackingGeometryMaker::collectModules_dd4hep(dd4hep::DetElement& detElement,
                                                  std::vector<dd4hep::DetElement> & modules) {
  const dd4hep::DetElement::Children& children = detElement.children();
  //std::cout<<__PRETTY_FUNCTION__<<" Collecting from "<<detElement.name()<<std::endl;
}

void TrackingGeometryMaker::collectSensors_dd4hep(dd4hep::DetElement& detElement,
                                                  std::vector<dd4hep::DetElement>& sensors) {

  if (debug_)
    std::cout<<__PRETTY_FUNCTION__<<" Collecting from " <<detElement.name()<<std::endl;
        
  const dd4hep::DetElement::Children& children = detElement.children();
        
  for (auto& child : children) {
    dd4hep::DetElement childDetElement = child.second;
    std::string childType = childDetElement.type();
            
    if (childType != "si_sensor")
      continue;
            
    //Check if there is an Acts extension associated to this detElement. If not, add it.
            
    Acts::ActsExtension* detExtension = nullptr;
    try {
      detExtension = childDetElement.extension<Acts::ActsExtension>();
      //std::cout<<detExtension->toString()<<std::endl;
    }
    catch (std::runtime_error& e) {
      //std::cout<<"Caught exception in "<<__PRETTY_FUNCTION__<<std::endl;
      //continue;
    }
            
    //Add the child if the detExtension is the TaggerTracker, the RecoilTracker or the Target(?)
    if ((detExtension!=nullptr)) {
      if (detExtension->hasType(childDetElement.name(),"si_sensor")){
        sensors.push_back(childDetElement);
      }
    }
    else {  //ActsExtension doesn't exist
      //std::cout<<__PRETTY_FUNCTION__<<"PF::DEBUG:: Adding the ActsExtension for sensor "<<childDetElement.name()<<std::endl;
      detExtension = new Acts::ActsExtension();
      detExtension->addType(childDetElement.name(), "si_sensor");
      childDetElement.addExtension<Acts::ActsExtension>(detExtension);
      sensors.push_back(childDetElement);
    }
  } // children loop
}// get sensors.

//Collect the subdetectors and add the ActsExtension to them    
//I expect to find the TaggerTracker and the RecoilTracker.
void TrackingGeometryMaker::collectSubDetectors_dd4hep(dd4hep::DetElement& detElement,
                                                       std::vector<dd4hep::DetElement>& subdetectors) {
  const dd4hep::DetElement::Children& children = detElement.children();
    
  //std::cout<<"Collecting from "<<detElement.name()<<std::endl;
        
  for (auto& child : children) {
    dd4hep::DetElement childDetElement = child.second;
    
    if (debug_) {
      std::cout<<"Child Name:: "<<childDetElement.name()<<std::endl;
      std::cout<<"Child Type:: "<<childDetElement.type()<<std::endl;
    }
    
    std::string childName = childDetElement.name();
    
    //Check here if I'm checking the TaggerTracker or the RecoilTracker. Skip the rest.
    if (
            childName != "TaggerTracker"  &&
            childName != "tagger_tracker" &&
            childName != "recoil_tracker" &&
            childName != "RecoilTracker") {
      continue;
    }
        
    //Check if an Acts extension is attached to the det Element (not necessary)
    Acts::ActsExtension* detExtension = nullptr;
    try {
      detExtension = childDetElement.extension<Acts::ActsExtension>();
      //std::cout<<detExtension->toString()<<std::endl;
    }
    catch (std::runtime_error& e) {
      //std::cout<<"Caught exception in "<<__PRETTY_FUNCTION__<<std::endl;
      //continue;
    }
        
    //Add the child if the detExtension is the TaggerTracker, the RecoilTracker or the Target(?)
    if ((detExtension!=nullptr)) {
      if (detExtension->hasType("TaggerTracker","detector") ||  
          detExtension->hasType("RecoilTracker","detector") ||
          detExtension->hasType("recoil_tracker","detector") ||
          detExtension->hasType("tagger_tracker","detector")) {
        subdetectors.push_back(childDetElement);
      }
    }
    else {  //ActsExtension doesn't exist
      //std::cout<<"PF::DEBUG:: Adding the ActsExtension to "<< childDetElement.name() << " " <<childDetElement.type() <<std::endl;
            
      detExtension = new Acts::ActsExtension();
      detExtension->addType(childDetElement.name(), "detector");
      childDetElement.addExtension<Acts::ActsExtension>(detExtension);
      detExtension->addType("axes", "definitions", "XYZ"); // no effect in changing this line for the sensors.
      subdetectors.push_back(childDetElement);
    }
        
    //recursive
    collectSubDetectors_dd4hep(childDetElement,subdetectors);
  }//children loop
}

void TrackingGeometryMaker::resolveSensitive(
    const dd4hep::DetElement& detElement,
    std::vector<std::shared_ptr<const Acts::Surface>>& surfaces,bool force) const {
  const dd4hep::DetElement::Children& children = detElement.children();
  
  if (!children.empty()) {
    for (auto& child : children) {
      dd4hep::DetElement childDetElement = child.second;
                      
      //Check material
      //std::cout<<childDetElement.volume().material().toString()<<std::endl;
                
      if (childDetElement.volume().isSensitive() || force) {
        //std::cout<<"isSensitive.. "<<std::endl;
        // create the surface
        surfaces.push_back(createSensitiveSurface(childDetElement));
      }
      resolveSensitive(childDetElement, surfaces,force);
    }
  }
}//resolve sensitive


std::shared_ptr<const Acts::Surface>
TrackingGeometryMaker::createSensitiveSurface(
    const dd4hep::DetElement& detElement) const {
  // access the possible extension of the DetElement
  Acts::ActsExtension* detExtension = nullptr;
  try {
    detExtension = detElement.extension<Acts::ActsExtension>();
  } catch (std::runtime_error& e) {
    //ACTS_WARNING("Could not get Acts::Extension");
    return nullptr;
  }
        
  //Axes orientations
  auto detAxis = detExtension->getType("axes", "definitions");

  //Add the material
  dd4hep::Material de_mat = detElement.volume().material();
  //std::cout<<childDetElement.volume().material().toString()<<std::endl;
  //std::cout<<"Silicon density "<<de_mat.density()<<std::endl;
  Acts::Material silicon = Acts::Material::fromMassDensity(de_mat.radLength(),de_mat.intLength(), de_mat.A(), de_mat.Z(), de_mat.density());
        
  //Get the thickness. The bounding box gives back half of the size in z. I scaled of factor 10 to bring it in mm. The detElement stores in cm units
  double thickness = 2*Acts::UnitConstants::cm*detElement.volume().boundingBox().z();
        
  Acts::MaterialSlab silicon_slab(silicon,thickness); 
  std::shared_ptr<Acts::HomogeneousSurfaceMaterial> homogeneous_mat = std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab);
        
  // Create the corresponding detector element !- memory leak --!
  //I've checked: cm is the right scalor to use since it picks the right units from the xml. 
  //Using cm here to multiply by 10. 
  Acts::DD4hepDetectorElement* dd4hepDetElement =
      new Acts::DD4hepDetectorElement(detElement, detAxis, Acts::UnitConstants::cm, 
                                      false, homogeneous_mat, nullptr); //is disc is always false.
        

  // return the surface
  return dd4hepDetElement->surface().getSharedPtr();
}

// I use UnitConstants::cm because the seems like that the dd4hep classes store the informations in cm
    
Acts::Transform3 TrackingGeometryMaker::convertTransform(
    const TGeoMatrix* tGeoTrans) const {
  // get the placement and orientation in respect to its mother
  const Double_t* rotation = tGeoTrans->GetRotationMatrix();
  const Double_t* translation = tGeoTrans->GetTranslation();

  return Acts::TGeoPrimitivesHelper::makeTransform(
      Acts::Vector3(rotation[0], rotation[3], rotation[6]),
      Acts::Vector3(rotation[1], rotation[4], rotation[7]),
      Acts::Vector3(rotation[2], rotation[5], rotation[8]),
      Acts::Vector3(translation[0] * Acts::UnitConstants::cm,
                    translation[1] * Acts::UnitConstants::cm,
                    translation[2] * Acts::UnitConstants::cm));
}


void TrackingGeometryMaker::getSurfaces(std::vector<const Acts::Surface*>& surfaces,
                                        std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry) {
  
  const Acts::TrackingVolume* tVolume = trackingGeometry->highestTrackingVolume();
  if (tVolume->confinedVolumes()) {
    for (auto volume : tVolume->confinedVolumes()->arrayObjects()) {
      if (volume->confinedLayers()) {
        for (const auto& layer : volume->confinedLayers()->arrayObjects()) {
          if (layer->layerType() == Acts::navigation) continue;
          for (auto surface : layer->surfaceArray()->surfaces()) {
            if (surface) {

              surfaces.push_back(surface);
                            
            }// surface exists
          } //surfaces
        }//layers objects
      }//confined layers
    }//volumes objects
  }//confined volumes
}

void TrackingGeometryMaker::testField(const std::shared_ptr<Acts::MagneticFieldProvider> bfield,
                                      const Acts::Vector3& eval_pos) {

  Acts::MagneticFieldProvider::Cache cache = bfield->makeCache(bctx_);
  std::cout<<"Pos::\n"<<eval_pos<<std::endl;
  std::cout<<" BField::\n"<<bfield->getField(eval_pos,cache).value() / Acts::UnitConstants::T <<std::endl;
  
}



void TrackingGeometryMaker::testMeasurmentCalibrator(const LdmxMeasurementCalibrator& calibrator,
                                                     const std::unordered_map<Acts::GeometryIdentifier, std::vector< ActsExamples::IndexSourceLink> > & map) {
  
  for (const auto& pair : map) {
    std::cout<<"GeometryID::"<<pair.first<<std::endl;
    for (auto& sl : pair.second) {
      calibrator.test(gctx_, sl);
    }
  }
}


//Tagger tracker: vol=2 , layer = [2,4,6,8,10,12,14], sensor=[1,2]
//Recoil tracker: vol=3 , layer = [2,4,6,8,10,12],    sensor=[1,2,3,4,5,6,7,8,9,10]

void TrackingGeometryMaker::makeLayerSurfacesMap(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry) {
  
  //loop over the tracking geometry to find all sensitive surfaces
  std::vector<const Acts::Surface*> surfaces;
  getSurfaces(surfaces, trackingGeometry);

  for (auto& surface : surfaces) {
    //std::cout<<"Check the surfaces"<<std::endl;
    //surface->toStream(gctx_,std::cout);
    //std::cout<<"GeometryID::"<<surface->geometryId()<<std::endl;
    //std::cout<<"GeometryID::"<<surface->geometryId().value()<<std::endl;
    
    //Layers from 1 to 14 - for the tagger 
    //unsigned int layerId = (surface->geometryId().layer() / 2) ;  // Old 1 sensor per layer

    unsigned int volumeId = surface->geometryId().volume();
    unsigned int layerId  = (surface->geometryId().layer() / 2); // set layer ID  from 1 to 7 for the tagger and from 1 to 6 for the recoil
    unsigned int sensorId = surface->geometryId().sensitive() - 1;   // set sensor ID from 0 to 1 for the tagger and from 0 to 9 for the axial sensors in the back layers of the recoil

    //surface ID = vol * 1000 + ly * 100 + sensor
    
    unsigned int surfaceId = volumeId * 1000 + layerId * 100 + sensorId;
    
    layer_surface_map_[surfaceId] = surface;
    
  }// surfaces loop

  
  if (debug_) {
    
    std::cout<<__PRETTY_FUNCTION__<<std::endl;
    
    for (auto const& surfaceId : layer_surface_map_) {
      std::cout<<getName()<<" "<< surfaceId.first<<std::endl;
      std::cout<<getName()<<" Check the surface"<<std::endl;
      surfaceId.second->toStream(gctx_,std::cout);
      std::cout<<getName()<<" GeometryID::"<<surfaceId.second->geometryId()<<std::endl;
      std::cout<<getName()<<" GeometryID::"<<surfaceId.second->geometryId().value()<<std::endl;
    }
  }
}

// This functioon takes the input parameters and makes the propagation for a simple event display

  bool TrackingGeometryMaker::WriteEvent(framework::Event &event,
					 const Acts::BoundTrackParameters& perigeeParameters,
					 const Acts::MultiTrajectory& mj,
					 const int &trackTip,
					 const std::vector<ldmx::LdmxSpacePoint*> ldmxsps) {  
  //Prepare the outputs..
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
  const auto evtLogger = Acts::getDefaultLogger("evtDisplay", Acts::Logging::INFO);
  Acts::PropagatorOptions<ActionList, AbortList> propagator_options(gctx_, bctx_, Acts::LoggerWrapper{*evtLogger});
  
  propagator_options.pathLimit = std::numeric_limits<double>::max();
  
  // Activate loop protection at some pt value
  propagator_options.loopProtection = false; //(startParameters.transverseMomentum() < cfg.ptLoopers);
  
  // Switch the material interaction on/off & eventually into logging mode
  auto& mInteractor = propagator_options.actionList.get<Acts::MaterialInteractor>();
  mInteractor.multipleScattering = true;
  mInteractor.energyLoss         = true;
  mInteractor.recordInteractions = false;

  // The logger can be switched to sterile, e.g. for timing logging
  auto& sLogger = propagator_options.actionList.get<Acts::detail::SteppingLogger>();
  sLogger.sterile = false;
  // Set a maximum step size
  propagator_options.maxStepSize = propagator_step_size_ * Acts::UnitConstants::mm;
  propagator_options.maxSteps    = propagator_maxSteps_;

  // Loop over the states and the surfaces of the multi-trajectory and get
  // the arcs of helix from start to next surface
  
  std::vector<Acts::BoundTrackParameters> prop_parameters;
  //std::vector<std::reference_wrapper<const Acts::Surface>> ref_surfaces;
  std::vector<std::reference_wrapper<const ActsExamples::IndexSourceLink>> sourceLinks;


  mj.visitBackwards(trackTip,[&](const auto& state) {
    auto typeFlags = state.typeFlags();
    
    //Only store the track states for each measurement
    if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
      const auto& surface = state.referenceSurface();
      //ref_surfaces.push_back(surface);
      Acts::BoundVector smoothed  = state.smoothed();
      //auto cov = state.smoothedCovariance;
      
      Acts::ActsScalar q = smoothed[Acts::eBoundQOverP] < 0 ? -1 * Acts::UnitConstants::e :
                           Acts::UnitConstants::e;
      
      prop_parameters.push_back(Acts::BoundTrackParameters(surface.getSharedPtr(),
                                                           smoothed,
                                                           q));
    }
  });
  
  
  //Reverse the parameters to start from the target
  std::reverse(prop_parameters.begin(), prop_parameters.end());

  //This holds all the steps to be merged
  std::vector<std::vector<Acts::detail::Step>> tmpSteps;
  tmpSteps.reserve(prop_parameters.size());
  
  if (debug_) {
    for (auto & params : prop_parameters) {
      std::cout<<getName()<<"::DEBUG::"<<std::endl;
      std::cout<<params.parameters()<<std::endl;
      std::cout<<params.position(gctx_)(0)<<std::endl;
    }
  }
  
  
  // Start from the first parameters
  // Propagate to next surface
  // Grab the next parameters
  // Propagate to the next surface..
  // The last parameters just propagate
  // TODO Fix double code
  // TODO Fix contorted code - directly push to the same vector

 

  std::vector<Acts::detail::Step> steps;

  //compute first the perigee to first surface:
  auto result = propagator_->propagate(perigeeParameters,
                                       prop_parameters.at(0).referenceSurface(),
                                       propagator_options);
  
  if (result.ok()) {
    const auto& resultValue = result.value();
    auto steppingResults =
        resultValue.template get<Acts::detail::SteppingLogger::result_type>();
    // Set the stepping result
    pOutput.first = std::move(steppingResults.steps);
    
    for (auto & step : pOutput.first)
      steps.push_back(std::move(step));
  }

  //follow now the trajectory
    
  
  for (int i_params = 0; i_params < prop_parameters.size(); i_params++) {

    if (i_params < prop_parameters.size() - 1) {
      
      auto result = propagator_->propagate(prop_parameters.at(i_params),
                                           prop_parameters.at(i_params+1).referenceSurface(),
                                           propagator_options);
      
      if (result.ok()) {
        const auto& resultValue = result.value();
        auto steppingResults =
            resultValue.template get<Acts::detail::SteppingLogger::result_type>();
        // Set the stepping result
        pOutput.first = std::move(steppingResults.steps);

        for (auto & step : pOutput.first)
          steps.push_back(std::move(step));
        
        // Record the propagator steps
        //tmpSteps.push_back(std::move(pOutput.first));
      }
    }
    
    //propagation for the last state
    else {
      auto result = propagator_->propagate(prop_parameters.at(i_params),
                                           propagator_options);
      
      if (result.ok()) {
        const auto& resultValue = result.value();
        auto steppingResults =
            resultValue.template get<Acts::detail::SteppingLogger::result_type>();
        // Set the stepping result
        pOutput.first = std::move(steppingResults.steps);

        for (auto & step : pOutput.first)
          steps.push_back(std::move(step));
        
        // Record the propagator steps
        //tmpSteps.push_back(std::move(pOutput.first));
      }
    }
  }

  
  //This holds all the steps to be merged
  //TODO Remove this and put all in the same vector directly in the for loop above

  /*
    
  int totalSize=0;

  for (auto & steps: tmpSteps) 
    totalSize+=steps.size();
  
    allSteps.reserve(totalSize);
  
  for (auto & steps: tmpSteps) 
    allSteps.insert(allSteps.end(), steps.begin(), steps.end());
  */
  
  propagationSteps.push_back(steps);
  
  
  writer_->WriteSteps(event,
                      propagationSteps,
                      ldmxsps);
}



} // namespace sim
} // namespace tracking

DECLARE_PRODUCER_NS(tracking::sim, TrackingGeometryMaker)
