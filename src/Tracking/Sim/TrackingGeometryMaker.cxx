#include "Tracking/Sim/TrackingGeometryMaker.h"
#include "Tracking/Sim/GeometryContainers.h"

#include "SimCore/Event/SimParticle.h"

//--- ACTS ---//
#include "Acts/Plugins/TGeo/TGeoPrimitivesHelper.hpp"

//--- DD4Hep ---//
#include "DD4hep/DetElement.h"

//--- C++ StdLib ---//
#include <iostream>
#include <type_traits>
#include <typeinfo>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif


namespace tracking {
namespace sim {
    
TrackingGeometryMaker::TrackingGeometryMaker(const std::string &name,
                                             framework::Process &process)
    : framework::Producer(name, process) {}

TrackingGeometryMaker::~TrackingGeometryMaker() {}

void TrackingGeometryMaker::onProcessStart() {

  detector_ = &detector();
  gctx_ = Acts::GeometryContext();
  bctx_ = Acts::MagneticFieldContext();
  
  // Get the world detector element
  dd4hep::DetElement world{detector_->world()};
  std::cout << "World volume name: " << world.name() << std::endl;
  Acts::CuboidVolumeBuilder cvb;
  std::vector<dd4hep::DetElement> subdetectors;

  //Get the ACTS Logger
  
  auto loggingLevel = Acts::Logging::VERBOSE;
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("DD4hepConversion", loggingLevel));
    
  //The subdetectors should be the TaggerTracker and the Recoil Tracker
  
  collectSubDetectors_dd4hep(world,subdetectors);
  std::cout<<"PF::DEBUG::"<<__PRETTY_FUNCTION__<<" size  of subdetectors::"<<subdetectors.size()<<std::endl;

  //loop over the subDetectors to gather all the configurations
  
  std::vector<Acts::CuboidVolumeBuilder::VolumeConfig> volBuilderConfigs;
  for (auto& subDetector : subdetectors) {
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
  
  std::cout<<"Tracking Geometry Builder... done"<<std::endl;
  
  Acts::TrackingGeometryBuilder tgb(tgbCfg);

  std::cout<<"Retrieving tracking geometry"<<std::endl;
  std::shared_ptr<const Acts::TrackingGeometry> tGeometry = 
      tgb.trackingGeometry(gctx_);
  
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
        objVis, *(tGeometry->highestTrackingVolume()),
        gctx_, containerView,
        volumeView, passiveView, sensitiveView, gridView,
        true,"",".");
  }


  //==> Move to a separate processor <== //

  //Generate a constant null magnetic field
  Acts::Vector3 b_field(0., 0., bfield_ * Acts::UnitConstants::T);

  std::cout<<"PF::DEBUG::BFIELD"<<std::endl;
  std::cout<<"=========="<<std::endl;
  std::cout<<b_field / Acts::UnitConstants::T<<std::endl;
  std::cout<<"=========="<<std::endl;
  
  std::shared_ptr<Acts::ConstantBField> bField = std::make_shared<Acts::ConstantBField>(b_field);

  //Setup the navigator
  Acts::Navigator::Config navCfg{tGeometry};
  navCfg.resolveMaterial   = true;
  navCfg.resolvePassive    = true;
  navCfg.resolveSensitive  = true;
  Acts::Navigator navigator(navCfg);

  //Setup the stepper (do a straight line first)
  //auto&& stepper = Acts::EigenStepper<>{std::move(bField)};
  //using Stepper = std::decay_t<decltype(stepper)>;

  auto&& stepper = Acts::EigenStepper<>{std::move(bField)};
  //using Propagator = Acts::Propagator<Stepper, Acts::Navigator>;
    
  //Setup the propagator
  propagator_ = std::make_shared<Propagator>(stepper, navigator);

    
  
  //Setup the propagator steps writer
  PropagatorStepWriter::Config cfg;
  cfg.filePath = steps_outfile_path_;

  writer_ = std::make_shared<PropagatorStepWriter>(cfg);


  //Create a mapping between the layers and the Acts::Surface
  makeLayerSurfacesMap(tGeometry);
}


void TrackingGeometryMaker::produce(framework::Event &event) {

  std::cout<<"PF ::DEBUG:: "<<__PRETTY_FUNCTION__<<std::endl;

  //1) Setup the actions and the abort list. For debugging add the Stepping Logger
  //2) Create the propagator options from them. If the magneticField context is empty it won't be used.
  //2a) If I want to pass a constant Field then use the Constant BField Class (inherits from the provider)
  
  //options_ = TestPropagatorOptions();
  
  //3) Get the steps vector and pass it to the root Writer -> in the event processor

  //Setup the starting - Should be done in configure -

  std::shared_ptr<const Acts::PerigeeSurface> perigee_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(perigee_location_.at(0), perigee_location_.at(1), perigee_location_.at(2)));

  // Output : the propagation steps
  std::vector<std::vector<Acts::detail::Step>> propagationSteps;

  propagationSteps.reserve(ntests_);
    
  //No randomisation

  /// d0 gaussian sigma
  double d0Sigma = d0sigma_ * Acts::UnitConstants::mm;
  /// z0 gaussian sigma
  double z0Sigma = z0sigma_ * Acts::UnitConstants::mm;
  /// phi gaussian sigma (used for covariance transport)
  double phiSigma = 0.0001;
  /// theta gaussian sigma (used for covariance transport)
  double thetaSigma = 0.0001;
  /// qp gaussian sigma (used for covariance transport)
  double qpSigma = 0.00001 / 1 * Acts::UnitConstants::GeV;
  /// t gaussian sigma (used for covariance transport)
  double tSigma = 1 * Acts::UnitConstants::ns;

  /// phi range - generate only in the X<0 plane
  uniform_phi_ = std::make_shared<std::uniform_real_distribution<double> >(phi_range_.at(0),
                                                                           phi_range_.at(1));

  /// theeta range - generate spanning the Z axis
  uniform_theta_ = std::make_shared<std::uniform_real_distribution<double> >(theta_range_.at(0),
                                                                             theta_range_.at(1));
  
  /// pt range
  std::pair<double, double> ptRange = {100 * Acts::UnitConstants::MeV,
    100 * Acts::UnitConstants::GeV};


  
  //Prepare the outputs..
  
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
  
  // execute the test for charged particles

  //Get the ACTS Logger -  Very annoying to have to define it in order to run this test.
  auto loggingLevel = Acts::Logging::VERBOSE;
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("LDMX Tracking Goemetry Maker", loggingLevel));
  
  PropagatorOptions propagator_options(gctx_, bctx_, Acts::LoggerWrapper{logger()});
  
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
  

  std::cout<<"PF::Running the propagator tests"<<std::endl;
  
  for (size_t it = 0; it < ntests_; ++it) {
    
    double d0     = d0Sigma * (*normal_)(generator_);
    double z0     = z0Sigma * (*normal_)(generator_);
    //double d0 = 0.;
    //double z0 = 0.;
    
    double phi    = (*uniform_phi_)(generator_);
    double theta = (*uniform_theta_)(generator_);
    double pt     = pt_ * Acts::UnitConstants::GeV;
    double p      = pt / sin(theta);
    double charge = -1.;
    double qop    = charge / p;
    double t      = 0.;
    
    
    // parameters

    Acts::BoundVector pars;
    pars << d0, z0, phi, theta, qop, t;
    
    
    Acts::Vector3 sPosition(0., 0., 0.);
    Acts::Vector3 sMomentum(0., 0., 0.);
    
    //no covariance transport
    auto cov = std::nullopt;
    
        
    // charged extrapolation - with hit recording
    Acts::BoundTrackParameters startParameters(perigee_surface, std::move(pars),
                                               std::move(cov));
    sPosition = startParameters.position(gctx_);
    sMomentum = startParameters.momentum();
    
    //run the propagator
    PropagationOutput pOutput;
    
    auto result   = propagator_->propagate(startParameters,propagator_options);
    
    if (result.ok()) {
      const auto& resultValue = result.value();
      auto steppingResults =
          resultValue.template get<Acts::detail::SteppingLogger::result_type>();
      // Set the stepping result
      pOutput.first = std::move(steppingResults.steps);
      
      //TODO:: ADD THE MATERIAL INTERACTION
      
      // Record the propagator steps
      propagationSteps.push_back(std::move(pOutput.first));
    }
    else
      std::cout<<"PF::ERROR::PROPAGATION RESULTS ARE NOT OK!!"<<std::endl;
  }//ntests_

  writer_->WriteSteps(event,propagationSteps);



  std::cout<<"Setting up the Kalman Filter Algorithm"<<std::endl;

  //#######################//
  //Kalman Filter algorithm//
  //#######################//

  //Step 1 - Form the source links
  
  std::vector<ActsExamples::IndexSourceLink> sourceLinks;
  //a) Loop over the sim Hits
  

  const std::vector<ldmx::SimTrackerHit> sim_hits  = event.getCollection<ldmx::SimTrackerHit>("TaggerSimHits");
  
  
  
  std::vector<ldmx::LdmxSpacePoint* > ldmxsps;

  //Convert to ldmxsps
  for (auto& simHit : sim_hits) {
    
    //Remove low energy deposit hits
    if (simHit.getEdep() >  0.05) {
      ldmxsps.push_back(utils::convertSimHitToLdmxSpacePoint(simHit));
    }
  }

  std::cout<<"ldmx points::"<<ldmxsps.size()<<std::endl;
  
  //The mapping between the geometry identifier and the IndexSourceLink that points to the hit
  std::unordered_map<Acts::GeometryIdentifier, std::vector< ActsExamples::IndexSourceLink> > geoId_sl_map_;
  std::unordered_multimap<Acts::GeometryIdentifier, ActsExamples::IndexSourceLink> geoId_sl_mmap_;
  
  //Check the hits associated to the surfaces
  for (unsigned int i_ldmx_hit = 0; i_ldmx_hit < ldmxsps.size(); i_ldmx_hit++) {

    ldmx::LdmxSpacePoint* ldmxsp = ldmxsps.at(i_ldmx_hit);
    unsigned int layerid = ldmxsp->layer();
    
    const Acts::Surface* hit_surface = layer_surface_map_[layerid];
    if (hit_surface) {
      std::cout<<"HIT "<<i_ldmx_hit<<" at layer"<<ldmxsp->layer()<<" is associated to Acts::surface::"<<hit_surface<<std::endl;
      ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(),i_ldmx_hit);
      geoId_sl_map_[hit_surface->geometryId()].push_back(idx_sl);

      geoId_sl_mmap_.insert(std::make_pair(hit_surface->geometryId(), idx_sl));
            
      //Transform the ldmx space point from global to local and store the information
      
      std::cout<<"Global hit position::"<<std::endl;
      std::cout<<ldmxsp->global_pos_<<std::endl;
      hit_surface->toStream(gctx_,std::cout);
      Acts::Vector3 dummy_momentum;
      
      //The 0.2 mm is to avoid entering in the case where the hit is too far from the surface -> try/catch here.
      Acts::Vector2 local_pos = hit_surface->globalToLocal(gctx_,ldmxsp->global_pos_,dummy_momentum, 0.2).value();
      
      ldmxsp->local_pos_ = local_pos;
      
      std::cout<<"Local hit position::"<<std::endl;
      std::cout<<ldmxsp->local_pos_<<std::endl;
      
    }
    else
      std::cout<<"HIT "<<i_ldmx_hit<<" at layer"<<(ldmxsps.at(i_ldmx_hit))->layer()<<" is not associated to any surface?!"<<std::endl;
    
  }
      
  //The generation informations
  //Gen 1 Momentum [MeV] X = 313.836
  //Gen 1 Momentum [MeV] Y = 0
  //Gen 1 Momentum [MeV] Z = 3987.67
  //Gen 1 Vertex [mm] X = -27.926
  //Gen 1 Vertex [mm] Y = 0
  //Gen 1 Vertex [mm] Z = -700
  

  //Truth based initial track parameters
  
  auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};;
  ldmx::SimParticle gen_e = particleMap[1];
  
  std::cout<<"PF::generatedElectron PID::"<<gen_e.getPdgID()<<std::endl;
  std::cout<<"PF::generatedElectron Energy::"<<gen_e.getEnergy()<<std::endl;


  Acts::Vector3 gen_e_pos{gen_e.getVertex().data()};
  Acts::Vector3 gen_e_mom{gen_e.getMomentum().data()};
  Acts::ActsScalar  gen_e_time = 0.;

  //Rotate to ACTS frame
  //z->x, x->y, y->z

  //(0 0 1) x  = z 
  //(1 0 0) y  = x
  //(0 1 0) z  = y

  Acts::SymMatrix3 acts_rot;
  acts_rot << 0,0,1,
      1,0,0,
      0,1,0;

  gen_e_pos = acts_rot * gen_e_pos;
  gen_e_mom = acts_rot * gen_e_mom;
  
  std::cout<<"PF::generatedElectron POS::"<<std::endl;
  std::cout<<gen_e_pos<<std::endl;

  std::cout<<"PF::generatedElectron MOM::"<<std::endl;
  std::cout<<gen_e_mom<<std::endl;

  Acts::FreeVector free_params;
  Acts::ActsScalar q = -1 * Acts::UnitConstants::e;
  Acts::ActsScalar p = gen_e_mom.norm();
  
  //Store the generated electron into track parameters to start the CKF
  free_params[Acts::eFreePos0]   = gen_e_pos(Acts::ePos0);
  free_params[Acts::eFreePos1]   = gen_e_pos(Acts::ePos1);
  free_params[Acts::eFreePos2]   = gen_e_pos(Acts::ePos2);
  free_params[Acts::eFreeTime]   = 0.;  
  free_params[Acts::eFreeDir0]   = gen_e_mom(0);
  free_params[Acts::eFreeDir1]   = gen_e_mom(1);
  free_params[Acts::eFreeDir2]   = gen_e_mom(2);
  free_params[Acts::eFreeQOverP] = (q != Acts::ActsScalar(0)) ? (q / p) : (1 / p);
  
  //Random test covariance
  Acts::FreeSymMatrix free_cov = 10. * Acts::FreeSymMatrix::Identity();

  std::cout<<free_cov<<std::endl;
  
  Acts::FreeTrackParameters gen_track_params(free_params, q, free_cov);
  
  std::cout<<"Check the position and momentum"<<std::endl;
  std::cout<<gen_track_params.position()<<std::endl;
  std::cout<<gen_track_params.momentum()<<std::endl;
  std::cout<<"Absolute momentum="<<gen_track_params.absoluteMomentum()<<std::endl;


  //The Kalman Filter needs to use bound trackParameters. Express the track parameters with respect the beamline at -700,0,0
  std::shared_ptr<const Acts::PerigeeSurface> gen_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(-700,-27.926, 0.0));

  //Transform the free parameters to the bound parameters
  auto bound_params =  Acts::detail::transformFreeToBoundParameters(free_params, *gen_surface, gctx_).value();

  Acts::BoundSymMatrix bound_cov = 10. * Acts::BoundSymMatrix::Identity();
  

  Acts::BoundTrackParameters gen_track_params_bound{gen_surface, bound_params, q, bound_cov};
  
  // ============   Setup the CKF  ============
  Acts::CombinatorialKalmanFilter<Propagator> ckf(*propagator_);  //Acts::Propagagtor<Acts::EigenStepper<>, Acts::Navigator>


  std::vector<Acts::BoundTrackParameters> startParameters = {gen_track_params_bound};

  
  Acts::GainMatrixUpdater kfUpdater;
  Acts::GainMatrixSmoother kfSmoother;

  // configuration for the measurement selector. Empty geometry identifier means applicable to all the detector
  // elements

  Acts::MeasurementSelector::Config measurementSelectorCfg = {
    // global default: no chi2 cut, only one measurement per surface
    {Acts::GeometryIdentifier(), {std::numeric_limits<double>::max(), 1u}},
  };

  /* -- main --
  Acts::MeasurementSelector::Config measurementSelectorCfg = {
    // global default: no chi2 cut, only one measurement per surface
    {Acts::GeometryIdentifier(),
     {{}, {std::numeric_limits<double>::max()}, {1u}}},
  };
  */
  
  Acts::MeasurementSelector measSel{measurementSelectorCfg};
  LdmxMeasurementCalibrator calibrator{ldmxsps};

  
  Acts::CombinatorialKalmanFilterExtensions ckf_extensions;
  ckf_extensions.calibrator.connect<&LdmxMeasurementCalibrator::calibrate>(&calibrator);
  ckf_extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(&kfUpdater);
  ckf_extensions.smoother.connect<&Acts::GainMatrixSmoother::operator()>(&kfSmoother);
  ckf_extensions.measurementSelector.connect<&Acts::MeasurementSelector::select>(&measSel);
  
  
  using LdmxSourceLinkAccessor_v2  = GeneralContainerAccessor<std::unordered_map<Acts::GeometryIdentifier, std::vector<ActsExamples::IndexSourceLink> > >;

  using LdmxSourceLinkAccessor_v3 = GeneralContainerAccessor<std::unordered_multimap<Acts::GeometryIdentifier, ActsExamples::IndexSourceLink> >  ;
  
  Acts::CombinatorialKalmanFilterOptions<LdmxSourceLinkAccessor<ActsExamples::IndexSourceLink> > kfOptions(
      gctx_,bctx_,cctx_,
      LdmxSourceLinkAccessor<ActsExamples::IndexSourceLink>(), ckf_extensions, Acts::LoggerWrapper{logger()},
      propagator_options,&(*perigee_surface));
  

  Acts::CombinatorialKalmanFilterOptions<LdmxSourceLinkAccessor_v2> kfOptions_v2(
      gctx_,bctx_,cctx_,
      LdmxSourceLinkAccessor_v2(), ckf_extensions, Acts::LoggerWrapper{logger()},
      propagator_options,&(*perigee_surface));

  Acts::CombinatorialKalmanFilterOptions<LdmxSourceLinkAccessor_v3> kfOptions_v3(
      gctx_,bctx_,cctx_,
      LdmxSourceLinkAccessor_v3(), ckf_extensions, Acts::LoggerWrapper{logger()},
      propagator_options,&(*perigee_surface));
  

  //Test the accessor
  
  std::cout<<"PF::TEST ACCESSOR V3::"<<std::endl;
  testAccessor(LdmxSourceLinkAccessor_v3(), geoId_sl_mmap_, layer_surface_map_[1]->geometryId());


  std::cout<<"PF::TEST ACCESSOR V1::"<<std::endl;
  testAccessor(LdmxSourceLinkAccessor<ActsExamples::IndexSourceLink>(), geoId_sl_map_, layer_surface_map_[1]->geometryId());

  // run the CKF for all initial track states
  //auto results = ckf.findTracks(geoId_sl_map_, startParameters, kfOptions);
  auto results = ckf.findTracks(geoId_sl_mmap_, startParameters, kfOptions_v3);
    
  
}

//A copy is not a good idea. TODO
Acts::CuboidVolumeBuilder::VolumeConfig TrackingGeometryMaker::volumeBuilder_dd4hep(dd4hep::DetElement& subdetector,Acts::Logging::Level logLevel) {
  
  //ACTS_INFO("Processing detector element:  " << subdetector.name());
  
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
  std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" the size of sensors="<<sensors.size()<<std::endl;
    
  std::vector<std::shared_ptr<const Acts::Surface>> surfaces;
    
  //Get all the sensitive surfaces for the tagger Tracker. 
  //For the moment I'm forcing to grep everything.
    
  resolveSensitive(subdetector,surfaces,true);
  std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" surfaces size::"<<surfaces.size()<<std::endl;
    
  //Check the surfaces that I created
            
  for (auto& surface : surfaces) {
    
    surface->toStream(gctx_,std::cout);
    std::cout<<std::endl;
    surface->surfaceMaterial()->toStream(std::cout);
    std::cout<<std::endl;
    std::cout<<surface->surfaceMaterial()->materialSlab(0,0).material().massDensity()<<std::endl;
    std::cout<<surface->surfaceMaterial()->materialSlab(0,0).material().molarDensity()<<std::endl;
  }
    
  //Surfaces configs
  std::vector<Acts::CuboidVolumeBuilder::SurfaceConfig> surfaceConfig;
  
  int counter = 0;
  for (auto& sensor : sensors) {
    counter++;

    Acts::CuboidVolumeBuilder::SurfaceConfig cfg;
        
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
        
    //Position and rotation of the surface
    std::cout<<cfg.position<<std::endl;
    std::cout<<cfg.rotation<<std::endl;
        
    //Get the bounds -  TODO..FIX hardcoding
    cfg.rBounds  = std::make_shared<const Acts::RectangleBounds>(Acts::RectangleBounds(20.17, 50));
    //cfg.rBounds  = std::make_shared<const Acts::RectangleBounds>(Acts::RectangleBounds(0.000002017, 0.00000050));
        
    // I don't think I need this to be defined.
    cfg.detElementConstructor = nullptr; 
        
    // Thickness
    double thickness = 2*Acts::UnitConstants::cm*sensor.volume().boundingBox().z();
                
    // Material
        
    dd4hep::Material de_mat = sensor.volume().material();
    Acts::Material silicon = Acts::Material::fromMassDensity(de_mat.radLength(),de_mat.intLength(), de_mat.A(), de_mat.Z(), de_mat.density());
    Acts::MaterialSlab silicon_slab(silicon,thickness); 
        
    cfg.thickness = thickness;
                
    cfg.surMat = std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab);
        

    surfaceConfig.push_back(cfg);
  }
    
  std::cout<<"Formed " <<surfaceConfig.size()<< " Surface configs"<<std::endl;
    
  //Layer Configurations
  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layerConfig;
    
  //One layer for surface...  TODO::Is this remotely right?
  //I'm not sure this is a good solution - check with Paul
  
  for (auto& sCfg : surfaceConfig) {
    Acts::CuboidVolumeBuilder::LayerConfig cfg;
    cfg.surfaceCfg = {sCfg};

    //The envelopes are distances (i.e. always positive)
    //To be fixed
    cfg.envelopeX = std::pair<double,double>{sCfg.thickness / 2. + 0.001, sCfg.thickness / 2. + 0.001};

    layerConfig.push_back(cfg);
    cfg.active = true;
  }
    
  std::cout<<"Formed " <<layerConfig.size()<< " layer configs"<<std::endl;
    
  //Create the volume

  std::cout<<"FORMING the boundaries for:"<<subdetector.name()<<std::endl;
  // Build the sub-detector volume configuration
  Acts::CuboidVolumeBuilder::VolumeConfig subDetVolumeConfig;
    
  // Get the transform wrt the world
  auto subDet_transform = convertTransform(&(subdetector.nominal().worldTransformation()));
    
  std::cout<<subDet_transform.translation()<<std::endl;
  std::cout<<subDet_transform.rotation()<<std::endl;


  //Rotate..Z->X, X->Y, Y->Z
  //Add 1 to not make it sit on the first layer surface
  Acts::Vector3 sub_det_position = {subDet_transform.translation()[2]-1, subDet_transform.translation()[0], subDet_transform.translation()[1]};
  
  
  double x_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().z()+1;
  double y_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().x();
  double z_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().y();
  

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
  
  
  return subDetVolumeConfig;
    
}

  
void TrackingGeometryMaker::configure(framework::config::Parameters &parameters) {
    
  // set dumping obj flag (integer for the moment) TODO
  dumpobj_            = parameters.getParameter<int>("dumpobj");
  steps_outfile_path_ = parameters.getParameter<std::string>("steps_file_path","propagation_steps.root");
  ntests_             = parameters.getParameter<int>("ntests", 1000);
  phi_range_          = parameters.getParameter<std::vector<double> >("phi_range",   {-1.1 * M_PI,-0.9 * M_PI});
  theta_range_        = parameters.getParameter<std::vector<double> >("theta_range", { 0.4 * M_PI, 0.6 * M_PI});
    
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);
  generator_.seed(std::chrono::system_clock::now().time_since_epoch().count());
  
  bfield_               = parameters.getParameter<double>("bfield", 0.);
  propagator_step_size_ = parameters.getParameter<double>("propagator_step_size", 200.);
  pt_                   = parameters.getParameter<double>("pt", 1.);
  d0sigma_              = parameters.getParameter<double>("d0sigma",1.);
  z0sigma_              = parameters.getParameter<double>("z0sigma",1.);
  perigee_location_ = parameters.getParameter<std::vector<double> >("perigee_location", {0.,0.,0.});
  
}


// **** //
//std::shared_ptr<const Acts::CylinderVolumeHelper> cuboidVolumeHelper_dd4hep(Logging::Level loggingLevel) {
//#####################//

void TrackingGeometryMaker::collectModules_dd4hep(dd4hep::DetElement& detElement,
                                                  std::vector<dd4hep::DetElement> & modules) {
  const dd4hep::DetElement::Children& children = detElement.children();
  std::cout<<__PRETTY_FUNCTION__<<" Collecting from "<<detElement.name()<<std::endl;
}

void TrackingGeometryMaker::collectSensors_dd4hep(dd4hep::DetElement& detElement,
                                                  std::vector<dd4hep::DetElement>& sensors) {
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
      std::cout<<detExtension->toString()<<std::endl;
    }
    catch (std::runtime_error& e) {
      //std::cout<<"Caught exception in "<<__PRETTY_FUNCTION__<<std::endl;
      //continue;
    }
            
    //Add the child if the detExtension is the TaggerTracker, the RecoilTracker or the Target(?)
    if ((detExtension!=nullptr)) {
      if (detExtension->hasType("si_sensor","detector")){
        sensors.push_back(childDetElement);
      }
    }
    else {  //ActsExtension doesn't exist
      std::cout<<__PRETTY_FUNCTION__<<"PF::DEBUG:: Adding the ActsExtension for sensor "<<childDetElement.name()<<std::endl;
      detExtension = new Acts::ActsExtension();
      detExtension->addType(childDetElement.name(), "si_sensor");
      childDetElement.addExtension<Acts::ActsExtension>(detExtension);
      //detExtension->addType("axes", "definitions", "XYZ"); //Issue?
      //detExtension->addType("axes", "definitions", "ZYX");
      sensors.push_back(childDetElement);
    }
            
  } // children loop
        
}// get sensors.

//Collect the subdetectors and add the ActsExtension to them    
//I expect to find the TaggerTracker and the RecoilTracker.
void TrackingGeometryMaker::collectSubDetectors_dd4hep(dd4hep::DetElement& detElement,
                                                       std::vector<dd4hep::DetElement>& subdetectors) {
  const dd4hep::DetElement::Children& children = detElement.children();
    
  std::cout<<"Collecting from "<<detElement.name()<<std::endl;
        
  for (auto& child : children) {
    dd4hep::DetElement childDetElement = child.second;
    std::cout<<"Child Name:: "<<childDetElement.name()<<std::endl;
    std::cout<<"Child Type:: "<<childDetElement.type()<<std::endl;
    std::string childName = childDetElement.name();
        
    //Check here if I'm checking the TaggerTracker or the RecoilTracker. Skip the rest.
    if (childName != "TaggerTracker"  &&
        childName != "tagger_tracker" &&
        childName != "recoil_tracker" &&
        childName != "RecoilTracker") {
      continue;
    }
        
    //Check if an Acts extension is attached to the det Element (not necessary)
    Acts::ActsExtension* detExtension = nullptr;
    try {
      detExtension = childDetElement.extension<Acts::ActsExtension>();
      std::cout<<detExtension->toString()<<std::endl;
    }
    catch (std::runtime_error& e) {
      //std::cout<<"Caught exception in "<<__PRETTY_FUNCTION__<<std::endl;
      //continue;
    }
        
    //Add the child if the detExtension is the TaggerTracker, the RecoilTracker or the Target(?)
    if ((detExtension!=nullptr)) {
      if (detExtension->hasType("TaggerTracker","detector") ||  
          detExtension->hasType("RecoilTracker","detector")) {
        subdetectors.push_back(childDetElement);
      }
    }
    else {  //ActsExtension doesn't exist
      std::cout<<"PF::DEBUG:: Adding the ActsExtension to "<< childDetElement.name() << " " <<childDetElement.type() <<std::endl;
            
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
  std::cout<<"resolving sensitive from "<<detElement.name()<<std::endl;
        
  if (!children.empty()) {
    std::cout<<"Resolving children.."<<std::endl;
    for (auto& child : children) {
      dd4hep::DetElement childDetElement = child.second;
      std::cout<<"childDetElement::"<<childDetElement.name()<< " "<<childDetElement.type()<<std::endl;
                
                
      //Check material
      std::cout<<childDetElement.volume().material().toString()<<std::endl;
                
      if (childDetElement.volume().isSensitive() || force) {
        std::cout<<"isSensitive.. "<<std::endl;
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
    std::cout<<"ERROR::"<<__PRETTY_FUNCTION__<<"Could not get Acts::Extension "<<std::endl;
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

template <typename source_link_accessor_t> 
void TrackingGeometryMaker::testAccessor(source_link_accessor_t accessor,
                                         const typename source_link_accessor_t::Container& container,
                                         const Acts::GeometryIdentifier& id) {

  accessor.container = &container;
  size_t nSourcelinks = accessor.count(id);
  std::cout<<"nSourceLinks == "<<(int)nSourcelinks<<std::endl;
  
  for (const auto& pair : container) {
    std::cout<<"GeometryID::"<<pair.first<<std::endl;
    std::cout<<"nSourceLinks == "<<(int)accessor.count(pair.first)<<std::endl;
    //std::cout << "decltype(container[key]) is " << type_name<decltype(container.at(pair.first))>() << '\n';
  }
  
  //Get all the source links on that surface
  auto [lower_it, upper_it] =
      accessor.range(id);
  
  for (auto it = lower_it; it != upper_it; ++it) {
    // get the source link
    const auto& sourceLink = accessor.at(it);

    std::cout<<"Type of it"<<std::endl;
    std::cout<<type_name<decltype(it)>()<<std::endl;
    
    std::cout<<"Type of at(it)::"<<std::endl;
    std::cout<<type_name<decltype(sourceLink)>()<<std::endl;
  }
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


void TrackingGeometryMaker::makeLayerSurfacesMap(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry) {
  
  //loop over the tracking geometry to find all sensitive surfaces
  std::vector<const Acts::Surface*> surfaces;
  getSurfaces(surfaces, trackingGeometry);

  std::cout<<"PF::CHECK THE SURFACES MADE IN THE TRACKING GEOMETRY"<<std::endl;

  for (auto& surface : surfaces) {
    std::cout<<"Check the surfaces"<<std::endl;
    surface->toStream(gctx_,std::cout);
    std::cout<<"GeometryID::"<<surface->geometryId()<<std::endl;
    std::cout<<"GeometryID::"<<surface->geometryId().value()<<std::endl;

    //Layers from 1 to 14
    unsigned int layerId = (surface->geometryId().layer() / 2);
    layer_surface_map_[layerId] = surface;
  }
  
}


template <typename T>
std::string
TrackingGeometryMaker::type_name()
{
  typedef typename std::remove_reference<T>::type TR;
  std::unique_ptr<char, void(*)(void*)> own
      (
#ifndef _MSC_VER
          abi::__cxa_demangle(typeid(TR).name(), nullptr,
                              nullptr, nullptr),
#else
          nullptr,
#endif
          std::free
       );
  std::string r = own != nullptr ? own.get() : typeid(TR).name();
  if (std::is_const<TR>::value)
    r += " const";
  if (std::is_volatile<TR>::value)
    r += " volatile";
  if (std::is_lvalue_reference<T>::value)
    r += "&";
  else if (std::is_rvalue_reference<T>::value)
    r += "&&";
  return r;
}





} // namespace sim
} // namespace tracking

DECLARE_PRODUCER_NS(tracking::sim, TrackingGeometryMaker)
