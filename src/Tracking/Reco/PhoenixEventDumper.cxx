#include "Tracking/Reco/PhoenixEventDumper.h"
#include "Tracking/Sim/TrackingUtils.h"

#include <fstream>

namespace tracking {
namespace reco {

PhoenixEventDumper::PhoenixEventDumper(const std::string& name, framework::Process& process)
    : TrackingGeometryUser(name,process) {}

void PhoenixEventDumper::onNewRun(const ldmx::RunHeader& rh) {

  tracking::sim::PropagatorStepWriter::Config writer_cfg;
  writer_cfg.filePath = "./propagation_steps.root";
  prop_writer_ = std::make_unique<tracking::sim::PropagatorStepWriter>(writer_cfg);
  

  //This should go to the Track Extrapolator

  // Setup a interpolated bfield map
  const auto map = std::make_shared<InterpolatedMagneticField3>(
      loadDefaultBField(field_map_,
                        default_transformPos,
                        default_transformBField));

  const auto stepper = Acts::EigenStepper<>{map};
  
  // Setup the navigator
  Acts::Navigator::Config navCfg{geometry().getTG()};
  navCfg.resolveMaterial = true;
  navCfg.resolvePassive = true;
  navCfg.resolveSensitive = true;
  navCfg.boundaryCheckLayerResolving = false;
  const Acts::Navigator navigator(navCfg);

  auto acts_loggingLevel = Acts::Logging::FATAL;
  propagator_ = std::make_unique<CkfPropagator>(stepper,
                                                navigator,
                                                Acts::getDefaultLogger("EVTDMPR",acts_loggingLevel));
  
  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(*propagator_,
                                                                       geometry_context(),
                                                                       magnetic_field_context());
  
  
}

void PhoenixEventDumper::configure(framework::config::Parameters &parameters) {

  field_map_ = parameters.getParameter<std::string>("field_map");
  
  TaggerTracks_ = parameters.getParameter<std::string>("taggerTracks","");
  RecoilTracks_ = parameters.getParameter<std::string>("recoilTracks","");

  TaggerMeasurements_ = parameters.getParameter<std::string>("taggerMeasurements","");
  RecoilMeasurements_ = parameters.getParameter<std::string>("recoilMeasurements","");

  EcalHits_           = parameters.getParameter<std::string>("EcalRecHits","");
  HcalHits_           = parameters.getParameter<std::string>("HcalRecHits","");
  
  eventnr_ = parameters.getParameter<int>("eventnr",1);
  
}

void PhoenixEventDumper::produce(framework::Event & event) {
  
  nevents_++;

  // Skip if it's not the event we want to dump
  if (nevents_ != eventnr_)
    return;
  
  if (!event.exists(TaggerMeasurements_)) {
    std::cout<<"ERROR::"+TaggerMeasurements_+" doesn't exist in the event";
    return;
  }
  
  if (!event.exists(RecoilMeasurements_)) {
    std::cout<<"ERROR::"+RecoilMeasurements_+" doesn't exist in the event";
    return;
  }
  

  if (!event.exists(EcalHits_)) {
    std::cout<<"ERROR::"<< EcalHits_ << " doesn't exist in the event";
    return;
  }

  if (!event.exists(HcalHits_)) {
    std::cout<<"ERROR:: HCalHits Collection "<< HcalHits_ << " doesn't exist in the event";
    return;
  }


  //if (!event.exists(TaggerTracks_)) {
  //  std::cout<<"ERROR::"+TaggerTracks_+" doesn't exist in the event";
  //      return;
  //}

  
  if (!event.exists(RecoilTracks_)) {
    std::cout<<"ERROR::"+TaggerTracks_+" doesn't exist in the event";
    return;
  }


  //Get the tracking geometry from the conditions database
  auto tg{geometry()};
  
  
  auto tms{event.getCollection<ldmx::Measurement>(TaggerMeasurements_)};
  auto rms{event.getCollection<ldmx::Measurement>(RecoilMeasurements_)};

  auto ecalHits{event.getCollection<ldmx::EcalHit>(EcalHits_)};
  auto hcalHits{event.getCollection<ldmx::HcalHit>(HcalHits_)};

  auto recoilTracks{event.getCollection<ldmx::Track>(RecoilTracks_)};
  

  std::cout<<"Found " + TaggerMeasurements_ + " with size " + tms.size()<<std::endl;
  std::cout<<"Found " + RecoilMeasurements_ + " with size " + rms.size()<<std::endl; 
  
  evtjson_ = {};
    
  evtjson_["LDMX_Event"]["event number"] = eventnr_;
  evtjson_["LDMX_Event"]["run number"]   = 0;
  
  float measSizeX = 2;
  float measSizeY = 2;
  float measSizeZ = 2;

  json jmeasurements_tagger = json::array();
  json jmeasurements_recoil = json::array();
  json jmeasurements_ecal   = json::array();
  json jmeasurements_hcal   = json::array();
  
  for (auto tmeas : tms) {

    //jsonf jmeas = jsonf::object({"type","Box"});
    auto meas_pos  = tmeas.getGlobalPosition();
    auto rot_meas_pos = meas_pos;

    // Rot back to LDMX Global
    rot_meas_pos[0] = meas_pos[1];
    rot_meas_pos[1] = meas_pos[2];
    rot_meas_pos[2] = meas_pos[0];
    
    json jmeas = json::object({{"type","Box"} , {"pos",{meas_pos}}});
    jmeasurements_tagger += jmeas;
  }

  for (auto rmeas : rms) {

    auto meas_pos  = rmeas.getGlobalPosition();
    std::vector<float> jloc{meas_pos[1],meas_pos[2],meas_pos[0],measSizeX,measSizeY,measSizeZ};
        
    json jmeas = json::object({{"type","Box"} , {"pos",jloc}});
    jmeasurements_recoil += jmeas;
  }


  // ECAL HITS
  
  for (auto ehit : ecalHits) {
    
    float posX   = ehit.getXPos();
    float posY   = ehit.getYPos();
    float posZ   = ehit.getZPos();
    
    float energy = ehit.getEnergy();
    
    std::vector<float> jloc{posX,posY,posZ, 5, 5, 5};
    
    //Define the position and the box size
    json jmeas = json::object({{"type","Box"}, {"pos",jloc}, {"color","0x3cb371"} });
    jmeasurements_ecal += jmeas;
    
  }


  //HCAL HITS

  for (auto hhit : hcalHits) {

    float posX = hhit.getXPos();
    float posY = hhit.getYPos();
    float posZ = hhit.getZPos();
    std::vector<float> jloc{posX,posY,posZ, 5, 5, 5};

    //Define the position and the box size
    json jmeas = json::object({{"type","Box"}, {"pos",jloc}, {"color","0x3cb371"} });
    jmeasurements_hcal += jmeas;
    
  }
  


  //Try to get tracks

  std::cout<<"Recoil tracks size "<< recoilTracks.size()<<std::endl;

  json jtracks_recoil = json::array();
  
  for (auto& trk : recoilTracks) {
    
    json jtrack = prepareTrack(event,
                               trk,
                               rms,
                               tg);
    
    jtracks_recoil.push_back(jtrack);
    
  }
  
  
  if (tms.size() > 0)
    evtjson_["LDMX_Event"]["Hits"]["TaggerHits"]   = {jmeasurements_tagger};
  
  if (rms.size() > 0)
    evtjson_["LDMX_Event"]["Hits"]["RecoilHits"]   = jmeasurements_recoil;
  
  if (ecalHits.size() > 0)
    evtjson_["LDMX_Event"]["Hits"]["EcalRecoHits"] = jmeasurements_ecal;


  if (hcalHits.size() > 0)
    evtjson_["LDMX_Event"]["Hits"]["HcalRecoHits"] = jmeasurements_hcal;

  
  if (recoilTracks.size() > 0)
    evtjson_["LDMX_Event"]["Tracks"]["RecoilTracks"]  = jtracks_recoil;
  
  std::ofstream file("test.json");
  file<<std::setw(4) << evtjson_ << '\n';
  
} //produce


json PhoenixEventDumper::prepareTrack(framework::Event& event,
                                      const ldmx::Track& track,
                                      const std::vector<ldmx::Measurement> measurements,
                                      const geo::TrackersTrackingGeometry&tg ) {

  //Prepare the outputs
  std::vector<std::vector<Acts::detail::Step>> propagationSteps;
  propagationSteps.reserve(1);

  //This holds the steps to be merged
  std::vector<std::vector<Acts::detail::Step>> tmpSteps;
  

  
  Acts::PropagatorOptions<ActionList, AbortList> pOptions(geometry_context(),
                                                          magnetic_field_context());
  pOptions.pathLimit = std::numeric_limits<double>::max();
  pOptions.loopProtection = false;
  auto& mInteractor = pOptions.actionList.get<Acts::MaterialInteractor>();
  mInteractor.multipleScattering = true;
  mInteractor.energyLoss         = true;
  mInteractor.recordInteractions = false;
  auto& sLogger = pOptions.actionList.get<Acts::detail::SteppingLogger>();
  sLogger.sterile = false;
  pOptions.maxStepSize = 10 * Acts::UnitConstants::mm;
  pOptions.maxSteps    = 100;
  
  
  /// Using some short hands for Recorded Material
  using RecordedMaterial = Acts::MaterialInteractor::result_type;
  using RecordedMaterialTrack =
      std::pair<std::pair<Acts::Vector3, Acts::Vector3>, RecordedMaterial>;

   /// Finally the output of the propagation test
  using PropagationOutput =
      std::pair<std::vector<Acts::detail::Step>, RecordedMaterial>;

  PropagationOutput pOutput;

  // Loop over the states and the surfaces of the multi-trajectory and get
  // the arcs of helix from start to next surface

  
  std::vector<Acts::BoundTrackParameters> prop_parameters;


  std::cout<<"Looping on track states:: "<< track.getTrackStates().size()<<std::endl;
  for (auto& ts : track.getTrackStates()) {

    //Skip the one at the target for the moment

    if (ts.ts_type != ldmx::TrackStateType::Meas)
      continue;
    
    //Get the surface from the tracking geometry
    std::cout<<"Getting the surface from the track state"<<std::endl;
    
    const Acts::Surface* ts_surface = tg.getSurface(ts.layerid);
    if (!ts_surface)
      std::cout<<"ERROR:: Retrieving the surface in PhoenixEventDumper"<<std::endl;

    std::cout<<"Forming the bound track state."<<std::endl;
    
    Acts::BoundVector smoothed;
    smoothed << 
        ts.params[0],
        ts.params[1],
        ts.params[2],
        ts.params[3],
        ts.params[4],
        ts.params[5];

    Acts::BoundSymMatrix covMat = tracking::sim::utils::unpackCov(ts.cov);
    
    prop_parameters.push_back(Acts::BoundTrackParameters(ts_surface->getSharedPtr(),
                                                         smoothed,
                                                         covMat));
    
    
  }// get track states

  //Dump something to check if I'm doing things right
  
  for (auto & params : prop_parameters) {
    std::cout<<(params.parameters()).transpose()<<std::endl;
    std::cout<<params.position(geometry_context()).transpose()<<std::endl;
    
  }

  // Start from the first parameters
  // Propagate to next surface
  // Grab the next parameters
  // Propagate to the next surface..
  // The last parameters just propagate
  
  
  std::vector<Acts::detail::Step> steps;

  
  //compute first the perigee to first surface:


  auto perigeeParameters = tracking::sim::utils::perigeeBoundParameters(track);
  auto result = propagator_->propagate(perigeeParameters,
                                       prop_parameters.at(0).referenceSurface(),
                                       pOptions);
  

  std::cout<<"Computed first extrapolation"<<std::endl;
  
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
                                           pOptions);

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
    
    else {
      auto result = propagator_->propagate(prop_parameters.at(i_params),
                                           pOptions);
      
      
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
  
  propagationSteps.push_back(steps);
  
  
  prop_writer_-> WriteSteps(event,
                            propagationSteps,
                            measurements,
                            Acts::Vector3(0.,0.,0.),
                            Acts::Vector3(1.,0.,0.)
                            );
  
  //This is called track by track, so only one track is coming out of here.
  
  return  prop_writer_->StepPosition(propagationSteps); 
  
} // prepare track

}
}


DECLARE_PRODUCER_NS(tracking::reco, PhoenixEventDumper)
