#include "Tracking/Sim/SeedFinderProcessor.h"

/* LDMX Seeding algorithm, based on Acts seeding algorithm */

namespace tracking{
  namespace sim {
    
    SeedFinderProcessor::SeedFinderProcessor(const std::string &name,
					     framework::Process &process) : 
      framework::Producer(name, process) {
      bottom_bin_finder_ = std::make_shared<Acts::BinFinder<ldmx::LdmxSpacePoint> > (
										     Acts::BinFinder<ldmx::LdmxSpacePoint>());
      
      top_bin_finder_ = std::make_shared<Acts::BinFinder<ldmx::LdmxSpacePoint> > (
										  Acts::BinFinder<ldmx::LdmxSpacePoint>());
      
    }
        
    SeedFinderProcessor::~SeedFinderProcessor() {}
        
    void SeedFinderProcessor::onProcessStart() {}
        
    void SeedFinderProcessor::configure(framework::config::Parameters &parameters) {
        
      //Default configuration
      
      //Tagger r max
      m_config.rMax = 1000.;
      m_config.deltaRMin = 3.; 
      m_config.deltaRMax = 220.; 
      m_config.collisionRegionMin = -50; 
      m_config.collisionRegionMax =  50; 
      m_config.zMin = -300;
      m_config.zMax = 300.;
      m_config.maxSeedsPerSpM = 5;
    
      //More or less the max angle is something of the order of 50 / 600 (assuming that the track hits all the layers)
      //Theta for the seeder is like ATLAS eta, so it's 90-lambda.
      //Max lamba is of the order of ~0.1 so cotThetaMax will be 1./tan(pi/2 - 0.1) ~ 1.4. 
      m_config.cotThetaMax = 1.5;

      //cotThetaMax and deltaRMax matter to choose the binning in z. The bin size is given by cotThetaMax*deltaRMax
    
      m_config.sigmaScattering = 2.25;
      m_config.minPt = 500.;
      m_config.bFieldInZ = 1.5e-3;  // in kT
      m_config.beamPos = {0, 0}; // units?
      m_config.impactMax = 20.;
    
    
      gridConf_.bFieldInZ = m_config.bFieldInZ;
      gridConf_.minPt = m_config.minPt;
      gridConf_.rMax = m_config.rMax;
      gridConf_.zMax = m_config.zMax;
      gridConf_.zMin = m_config.zMin;
      gridConf_.deltaRMax = m_config.deltaRMax;
      gridConf_.cotThetaMax = m_config.cotThetaMax;


      //The seed finder needs a seed filter instance
      //In the seed finder there is the correction for the beam axis, which you could ignore if you set the penalty for high impact parameters. So removing that in the seeder config.

      //Seed filter configuration
      m_seedFilter_cfg.impactWeightFactor = 0.;
    
      //For the moment no experiment dependent cuts are assigned to the filter
      m_config.seedFilter = std::make_unique<Acts::SeedFilter<ldmx::LdmxSpacePoint>>(
										     Acts::SeedFilter<ldmx::LdmxSpacePoint>(m_seedFilter_cfg));

      seed_finder_ = std::make_shared<Acts::Seedfinder<ldmx::LdmxSpacePoint> >(m_config);
            
    }
        
    void SeedFinderProcessor::produce(framework::Event &event) {
            
      auto start = std::chrono::high_resolution_clock::now();
            
      m_nevents++;
            
      //Read in the Sim hits -- TODO choose which collection from config
            
      const std::vector<ldmx::SimTrackerHit> simHits = event.getCollection<ldmx::SimTrackerHit>("TaggerSimHits");

      std::vector<ldmx::LdmxSpacePoint* > ldmxsps;
            
      //Only convert simHits that have at least 0.05 edep
                        
      for (auto& simHit : simHits) {
                
	//Remove weirdly simulated hits?
	if (simHit.getEdep() >  0.05) {
	  ldmxsps.push_back(convertSimHitToLdmxSpacePoint(simHit));
	}
      }    
      
      //TODO:: I think here is not necessary to only select 3 points. 
      //Select the ones to be used for seeding
      //I'll use layer 3,5,7. I also filter out here the space points that are not matched with electrons (using the pID)
      
      
      std::vector<const ldmx::LdmxSpacePoint*> spVec;

      for (size_t isp = 0; isp < ldmxsps.size(); isp++) {
        
	if (ldmxsps[isp]->layer()==3 || ldmxsps[isp]->layer()==7 || ldmxsps[isp]->layer()==9) 
	  spVec.push_back(ldmxsps[isp]);
      }

      //covariance tool: returns the covariance from the space point
      auto covariance_tool = [=](const ldmx::LdmxSpacePoint& sp, float, float,
				 float) -> std::pair<Acts::Vector3, Acts::Vector2> {
	Acts::Vector3 position{sp.x(), sp.y(), sp.z()};
	Acts::Vector2 covariance{sp.varianceR(), sp.varianceZ()};
	return std::make_pair(position, covariance);
      };



      std::unique_ptr<Acts::SpacePointGrid<ldmx::LdmxSpacePoint> > grid = Acts::SpacePointGridCreator::createGrid<ldmx::LdmxSpacePoint>(gridConf_);
      
      // create the space point group
      auto spGroup = Acts::BinnedSPGroup<ldmx::LdmxSpacePoint>(
							       spVec.begin(), spVec.end(),
							       covariance_tool,
							       bottom_bin_finder_, top_bin_finder_,
							       std::move(grid), m_config);
      
      // seed vector
      using SeedContainer = std::vector<Acts::Seed<ldmx::LdmxSpacePoint>>;
      SeedContainer seeds;
      seeds.clear();

      // find the seeds
      auto group = spGroup.begin();
      auto group_end = spGroup.end();
      
      for (; !(group == group_end); ++group) {
	const auto& groupSeeds =
	  seed_finder_->createSeedsForGroup(group.bottom(), group.middle(), group.top());
	std::copy(groupSeeds.begin(), groupSeeds.end(), std::back_inserter(seeds));
      }
      
      int numSeeds = seeds.size();
      
      for (auto& seed : seeds) {
	
	auto ldmxspvec = seed.sp();
	  
	//Acts::Vector2 refPoint{0.,0.};
	  
	
	//Fit the seeds and extract the track parameters
	//std::array<double,9> outData;
	//m_seedToTrackMaker->KarimakiFit(ldmxspvec,outData,refPoint);
	//std::cout<<outData[0]<< " " <<m_config.bFieldInZ<<std::endl;
	//std::cout<<"momentum::" << (1/outData[0]) * (300. * m_config.bFieldInZ) <<std::endl;
        
	//Acts::Transform3D tP;
        
	//tP.setIdentity();
        
	//tP(0,0)=0;
	//tP(0,1)=0;
	//tP(0,2)=1;
	
	//tP(1,0)=0;
	//tP(1,1)=1;
	//tP(1,2)=0;
	
	//tP(2,0)=1;
	//tP(2,1)=0;
	//tP(2,2)=0;
        
	//tP.translation().x() = 0.;
	//tP.translation().y() = 0.;
	//tP.translation().z() = 0.;
        
	//std::cout<<"PF:: CHECK CHECK\n"<<tP.translation()<<std::endl;
        
	//m_seedToTrackMaker->FitSeedAtlas(ldmxspvec,outData,tP,0.0015);
        
	//std::cout<<outData[0]<<" "<<outData[1]<<" "<<outData[2]<<" "<<outData[3]<<" "<<outData[4]<<std::endl;
      }
          
      //std::cout<<spVec.size() << " hits, " << seedVector.size() << " regions, "
      std::cout<< numSeeds << " seeds"<<std::endl;
      
      auto end = std::chrono::high_resolution_clock::now();
    
      //long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
    
      auto diff = end-start;
    
      m_processingTime += std::chrono::duration <double, std::milli> (diff).count();
    
    
    }//produce
  
    void SeedFinderProcessor::onProcessEnd() { 
    
      std::cout<<"PROCESSOR:: "<<this->getName()<<"   AVG Time/Event: " <<m_processingTime  / m_nevents << " ms"<<std::endl;
    
    }
  
  
    //This method converts a SimHit in a LdmxSpacePoint for the Acts seeder.
    // (1) Rotate the coordinates into acts::seedFinder coordinates defined by B-Field along z axis [Z_ldmx -> X_acts, X_ldmx->Y_acts, Y_ldmx->Z_acts]
    // (2) Saves the error information. At the moment the errors are fixed. They should be obtained from the digitized hits.
  
    //TODO::Move to shared pointers?!
    ldmx::LdmxSpacePoint* SeedFinderProcessor::convertSimHitToLdmxSpacePoint(const ldmx::SimTrackerHit& hit) {
            
      std::vector<float> sim_hit_pos = hit.getPosition();
            
      
      //This is in the transverse plane
      float sigma_rphi = 0.25;  //250um
            
      //This is in the direction along the b-field
      float sigma_z    = 0.50;  //50 um
      
      float ldmxsp_x = sim_hit_pos[2];
      float ldmxsp_y = sim_hit_pos[0];
      float ldmxsp_z = sim_hit_pos[1];
      
      
      return new ldmx::LdmxSpacePoint(ldmxsp_x, ldmxsp_y,ldmxsp_z,
				      hit.getTime(), hit.getLayerID(), 
				      sigma_rphi*sigma_rphi, sigma_z*sigma_z,
				      hit.getID());
      
    }
                
  }// namespace sim
} //namespace tracking


DECLARE_PRODUCER_NS(tracking::sim, SeedFinderProcessor)
