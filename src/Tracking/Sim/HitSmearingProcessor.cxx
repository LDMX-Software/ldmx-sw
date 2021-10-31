#include "Tracking/Sim/HitSmearingProcessor.hpp"

using namespace framework;

namespace tracking{
  namespace sim {
    
    HitSmearingProcessor::HitSmearingProcessor(const std::string &name,
					       framework::Process &process)
      : framework::Producer(name, process) {}

    HitSmearingProcessor::~HitSmearingProcessor() {}

    void HitSmearingProcessor::onProcessStart() {
    
      distN = std::make_shared<std::normal_distribution<double> >(0.,1.);
    
    }


    void HitSmearingProcessor::configure(framework::config::Parameters &parameters) {
          
      //Default configuration
      m_inputHitCollection = "TaggerSimHits";
      m_outputHitCollection = "SmearedTaggerSimHits";
      
    }
        
    void HitSmearingProcessor::produce(framework::Event &event) {
            
      //Read in the Sim hits -- TODO choose which collection from config

      std::cout<<"Hit Collection IN: "<<m_inputHitCollection<<std::endl;
      std::cout<<"Hit Collection OUT: "<<m_outputHitCollection<<std::endl;
            
      const std::vector<ldmx::SimTrackerHit> simHits = event.getCollection<ldmx::SimTrackerHit>("TaggerSimHits");
      
      for (auto& simHit : simHits) {
	simHit.Print();
      }
      
    }//produce
        
    
    //This method converts a SimHit in a LdmxSpacePoint for the Acts seeder. It smears the SimHit according to a gaussian distribution. The seed is fixed.
    // (1) Rotate the coordinates into acts::seedFinder coordinates defined by B-Field along z axis [Z_ldmx -> X_acts, X_ldmx->Y_acts, Y_ldmx->Z_acts]
    // (2) Smear  the location of the simulated hits and store the smeared hits into LdmxSpacepoints, saving the error information.
        

    //sigma_u and sigma_v are more sensitive and less sensitive directions.
        
    std::shared_ptr<ldmx::SimTrackerHit> HitSmearingProcessor::smearSimHit(const ldmx::SimTrackerHit& hit) {
            
      std::vector<float> sim_hit_pos = hit.getPosition();

      float sigma_u = 0.05; //50um
      float sigma_v = 0.25; //250um

      float ldmxsp_x = sim_hit_pos[2];
      float ldmxsp_y = sim_hit_pos[0];
      float ldmxsp_z = sim_hit_pos[1];

      std::cout<<"PF:: DEBUG:: Pre-smearing::(x,y,z) "<<ldmxsp_x<<" " <<ldmxsp_y<<" "<<ldmxsp_z<<std::endl;
      
      std::default_random_engine generator;
      std::normal_distribution<float> distribution(0.0, 1.0);
      
      float smear_factor = distribution(generator);
            
      //double smear_factor = distN(m_def_random_engine);
      ldmxsp_y = ldmxsp_y + smear_factor*sigma_v;
            
      smear_factor = distribution(generator);
      ldmxsp_z = ldmxsp_z + smear_factor*sigma_u;

      std::cout<<"PF:: DEBUG:: After-smearing::(x,y,z) "<<ldmxsp_x<<" " <<ldmxsp_y<<" "<<ldmxsp_z<<std::endl;
      
    }
    
  }// namespace sim
} //namespace tracking


DECLARE_PRODUCER_NS(tracking::sim, HitSmearingProcessor)
