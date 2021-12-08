#include "Tracking/Sim/SeedFinderProcessor.h"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"



/* This processor takes in input a set of 3D space points and builds seedTracks using the ACTS algorithm
 * which is based on the ATLAS 3-space point conformal fit.
 * 
 */


namespace tracking{
namespace sim {
    
SeedFinderProcessor::SeedFinderProcessor(const std::string &name,
                                         framework::Process &process) : 
    framework::Producer(name, process) {

  //Should be the typical behaviour.
  int numPhiNeighbors = 1;
  std::vector<std::pair<int, int>> zBinNeighborsTop;
  std::vector<std::pair<int, int>> zBinNeighborsBottom;
  
  
  bottom_bin_finder_ = std::make_shared<Acts::BinFinder<ldmx::LdmxSpacePoint> > (
      Acts::BinFinder<ldmx::LdmxSpacePoint>(zBinNeighborsBottom,numPhiNeighbors));
      
  top_bin_finder_ = std::make_shared<Acts::BinFinder<ldmx::LdmxSpacePoint> > (
      Acts::BinFinder<ldmx::LdmxSpacePoint>(zBinNeighborsTop,numPhiNeighbors));

  seed_to_track_maker_ = std::make_shared<tracking::sim::SeedToTrackParamMaker>();
  
}
        
SeedFinderProcessor::~SeedFinderProcessor() {}
        
void SeedFinderProcessor::onProcessStart() {}
        
void SeedFinderProcessor::configure(framework::config::Parameters &parameters) {
        
  //Default configuration
      
  //Tagger r max
  config_.rMax = 1000.;
  config_.deltaRMin = 3.; 
  config_.deltaRMax = 220.; 
  config_.collisionRegionMin = -50; 
  config_.collisionRegionMax =  50; 
  config_.zMin = -300;
  config_.zMax = 300.;
  config_.maxSeedsPerSpM = 5;
    
  //More or less the max angle is something of the order of 50 / 600 (assuming that the track hits all the layers)
  //Theta for the seeder is like ATLAS eta, so it's 90-lambda.
  //Max lamba is of the order of ~0.1 so cotThetaMax will be 1./tan(pi/2 - 0.1) ~ 1.4. 
  config_.cotThetaMax = 1.5;

  //cotThetaMax and deltaRMax matter to choose the binning in z. The bin size is given by cotThetaMax*deltaRMax
    
  config_.sigmaScattering = 2.25;
  config_.minPt           = 500. * Acts::UnitConstants::MeV;  
  config_.bFieldInZ       = 1.44 * Acts::UnitConstants::T;
  config_.beamPos         = {0, 0}; // units mm ?
  config_.impactMax       = 20.;
    
    
  grid_conf_.bFieldInZ       = config_.bFieldInZ;
  grid_conf_.minPt           = config_.minPt;
  grid_conf_.rMax            = config_.rMax;
  grid_conf_.zMax            = config_.zMax;
  grid_conf_.zMin            = config_.zMin;
  grid_conf_.deltaRMax       = config_.deltaRMax;
  grid_conf_.cotThetaMax     = config_.cotThetaMax;
  grid_conf_.numPhiNeighbors = 1.;



  //The seed finder needs a seed filter instance
  //In the seed finder there is the correction for the beam axis, which you could ignore if you set the penalty for high impact parameters. So removing that in the seeder config.

  //Seed filter configuration
  seed_filter_cfg_.impactWeightFactor = 0.;
    
  //For the moment no experiment dependent cuts are assigned to the filter
  config_.seedFilter = std::make_unique<Acts::SeedFilter<ldmx::LdmxSpacePoint>>(
      Acts::SeedFilter<ldmx::LdmxSpacePoint>(seed_filter_cfg_));

  seed_finder_ = std::make_shared<Acts::Seedfinder<ldmx::LdmxSpacePoint> >(config_);

  //In ACTS coordinates and BField in Tesla
  bField_(0)=0.;
  bField_(1)=0.;
  bField_(2)=config_.bFieldInZ;
  
}
        
void SeedFinderProcessor::produce(framework::Event &event) {
            
  auto start = std::chrono::high_resolution_clock::now();
            
  nevents_++;
            
  //Read in the Sim hits -- TODO choose which collection from config
            
  const std::vector<ldmx::SimTrackerHit> sim_hits = event.getCollection<ldmx::SimTrackerHit>("TaggerSimHits");

  std::vector<ldmx::LdmxSpacePoint* > ldmxsps;
            
  //Only convert simHits that have at least 0.05 edep


  std::cout<<"Converting sim hits"<<std::endl;
                        
  for (auto& simHit : sim_hits) {
                
    //Remove low energy deposit hits
    if (simHit.getEdep() >  0.05) {
      ldmxsps.push_back(utils::convertSimHitToLdmxSpacePoint(simHit));
    }
  }    
  std::cout<<"Converted "<< ldmxsps.size() << " hits "<<std::endl;
  
  //TODO:: For the moemnt I am only selecting 3 points. In principle the grid should be able to find all combinatorics
  //I'll use layer 3,5,7.

  std::vector<const ldmx::LdmxSpacePoint*> spVec;

  for (size_t isp = 0; isp < ldmxsps.size(); isp++) {
        
    if (ldmxsps[isp]->layer()==3 || ldmxsps[isp]->layer()==7 || ldmxsps[isp]->layer()==9) 
      spVec.push_back(ldmxsps[isp]);
  }

  std::cout<<"Will use spVec::"<<spVec.size()<<" hits "<<std::endl;

  
  //covariance tool: returns the covariance from the space point
  auto covariance_tool = [=](const ldmx::LdmxSpacePoint& sp, float, float,
                             float) -> std::pair<Acts::Vector3, Acts::Vector2> {
    Acts::Vector3 position{sp.x(), sp.y(), sp.z()};
    Acts::Vector2 covariance{sp.varianceR(), sp.varianceZ()};
    return std::make_pair(position, covariance);
  };

  std::cout<<"Creating the grid"<<std::endl;
  
  std::unique_ptr<Acts::SpacePointGrid<ldmx::LdmxSpacePoint> > grid = Acts::SpacePointGridCreator::createGrid<ldmx::LdmxSpacePoint>(grid_conf_);

  std::cout<<"Creating the space point group"<<std::endl;
  
  // create the space point group
  auto spGroup = Acts::BinnedSPGroup<ldmx::LdmxSpacePoint>(
      spVec.begin(), spVec.end(),
      covariance_tool,
      bottom_bin_finder_, top_bin_finder_,
      std::move(grid), config_);


  std::cout<<"Seed vector"<<std::endl;
  
  // seed vector
  using SeedContainer = std::vector<Acts::Seed<ldmx::LdmxSpacePoint>>;
  SeedContainer seeds;
  seeds.clear();

  Acts::Seedfinder<ldmx::LdmxSpacePoint>::State state;
  
  std::cout<<"finding seeds"<<std::endl;
  
  // find the seeds
  auto group = spGroup.begin();
  auto group_end = spGroup.end();
  
  for (; !(group == group_end); ++group) {
    seed_finder_->createSeedsForGroup(state, std::back_inserter(seeds), group.bottom(),
                                      group.middle(), group.top());
  }
  
  int numSeeds = seeds.size();
  
  for (auto& seed : seeds) {
    
    auto ldmxspvec = seed.sp();
    
    //Fit the seeds and extract the track parameters 
    
    //Local Surface (u,v,w) frame (wrt ACTS frame) rotation
    Acts::Vector3 uaxis{0,0,1};
    Acts::Vector3 vaxis{0,-1,0};
    Acts::Vector3 waxis{1,0,0};
        
    Acts::RotationMatrix3 s_rotation;
    s_rotation.col(0) = uaxis;
    s_rotation.col(1) = vaxis;
    s_rotation.col(2) = waxis;

    //Translation to the reference plane origin
    //TODO:: Attach the space points to the surfaces and provide the origin on surface.
    
    //Acts::Vector3 surface_origin{0.,0.,0.};
    Acts::Translation3 s_trans(0.,0.,0.);
    
    //Build the local to global transform 
    Acts::Transform3 tP(s_trans * s_rotation);
    
    //TODO:: Correct the 6th parameter (time)
    auto params = seed_to_track_maker_->estimateTrackParamsFromSeed(tP,
                                                                    ldmxspvec.begin(),ldmxspvec.end(),
                                                                    bField_, 1. * Acts::UnitConstants::T, 0.5);
    
    
    std::cout<<(*params)[Acts::eBoundLoc0]<<" "
             <<(*params)[Acts::eBoundLoc1]<<" "
             <<(*params)[Acts::eBoundPhi]<< " "
             <<(*params)[Acts::eBoundTheta]<<" "
             <<(*params)[Acts::eBoundQOverP]<< " "
             <<(*params)[Acts::eBoundTime]
             <<std::endl;
  }
  
  auto end = std::chrono::high_resolution_clock::now();
  
  //long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  
  auto diff = end-start;
    
  processing_time_ += std::chrono::duration <double, std::milli> (diff).count();
    
    
}//produce
  
void SeedFinderProcessor::onProcessEnd() { 
    
  std::cout<<"PROCESSOR:: "<<this->getName()<<"   AVG Time/Event: " <<processing_time_ / nevents_ << " ms"<<std::endl;
    
}
  
  
} //namespace sim
} //namespace tracking


DECLARE_PRODUCER_NS(tracking::sim, SeedFinderProcessor)
