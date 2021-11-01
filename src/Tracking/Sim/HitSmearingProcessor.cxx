/**
 * @file HitSmearingProcessor.cxx
 * @brief Class that performs simulated hit smearing in the tracker
 * @author PF, SLAC National Accelerator Laboratory
 */


#include "Tracking/Sim/HitSmearingProcessor.hpp"

using namespace framework;

namespace tracking{
  namespace sim {
    
    HitSmearingProcessor::HitSmearingProcessor(const std::string &name,
					       framework::Process &process)
      : framework::Producer(name, process) {}

    HitSmearingProcessor::~HitSmearingProcessor() {}

    void HitSmearingProcessor::onProcessStart() {
    
      distN = std::make_shared<std::normal_distribution<float> >(0.,1.);
    
    }


    void HitSmearingProcessor::configure(framework::config::Parameters &parameters) {
          
      //Default configuration
      m_inputHitCollections = parameters.getParameter<std::vector<std::string> >("inputHitCollections");
      m_outputHitCollections = parameters.getParameter<std::vector<std::string> >("outputHitCollections");

      m_taggerSigma_u = parameters.getParameter<double>("taggerSigma_u",0.05); 
      m_taggerSigma_v = parameters.getParameter<double>("taggerSigma_v",0.25); 
      
      m_recoilSigma_u = parameters.getParameter<double>("recoilSigma_u",0.05); 
      m_recoilSigma_v = parameters.getParameter<double>("recoilSigma_v",0.25); 
      
    }
        
    void HitSmearingProcessor::produce(framework::Event &event) {
            
      if (m_inputHitCollections.size() != m_outputHitCollections.size()) {
	std::cout<<"ERROR::Size of the collections are different::"<<m_inputHitCollections.size()<<"!="<<m_outputHitCollections.size()<<std::endl;
	return;
      }
      
      for (unsigned int i_coll=0; i_coll<m_inputHitCollections.size(); i_coll++) {

	const std::vector<ldmx::SimTrackerHit> simHits = event.getCollection<ldmx::SimTrackerHit>(m_inputHitCollections[i_coll]);
	
	std::vector<ldmx::SimTrackerHit> smearedHits;

	for (auto& simHit : simHits) {
	  
	  smearedHits.push_back(smearSimHit(simHit));
	  
	}
	event.add(m_outputHitCollections[i_coll], smearedHits);
      }

    }//produce
    
    //This method smears the SimHit according to two independent gaussian distributions in u and v direction.
    //Tagger and Recoil hits can be smeared with different sigma factors. The hits generated in the two detectors
    //are distinguished by checking the location along the beam.
        
    ldmx::SimTrackerHit HitSmearingProcessor::smearSimHit(const ldmx::SimTrackerHit& hit) {
    
      std::vector<float> sim_hit_pos = hit.getPosition();
      ldmx::SimTrackerHit smeared_hit;
    
      float sigma_u = m_taggerSigma_u;
      float sigma_v = m_taggerSigma_v;
    
      //Check if the sim hit is in the tagger or in the recoil to choose the smearing factor.
      if (sim_hit_pos[2] > 0 ) {
	sigma_u = m_recoilSigma_u;
	sigma_v = m_recoilSigma_v;
      }
      
      //LDMX Global X, along the less sensitive direction
      float smear_factor = (*distN)(m_generator);
      sim_hit_pos[0] += smear_factor*sigma_v;
            
      //LDMX Global Y, along the sensitive direction
      smear_factor = (*distN)(m_generator);
      sim_hit_pos[1] += smear_factor*sigma_u;

      //Fill the smeared hit
      
      //The ID will be the same
      smeared_hit.setID(hit.getID());  

      smeared_hit.setLayerID(hit.getLayerID()); 
      smeared_hit.setModuleID(hit.getModuleID()); 
      smeared_hit.setPosition(sim_hit_pos[0],sim_hit_pos[1],sim_hit_pos[2]); 
      smeared_hit.setEdep(hit.getEdep());
      smeared_hit.setEnergy(hit.getEnergy());
      smeared_hit.setTime(hit.getTime());

      //Change path-length will be the same
      smeared_hit.setPathLength(hit.getPathLength());
      
      smeared_hit.setMomentum(hit.getMomentum()[0],hit.getMomentum()[1],hit.getMomentum()[2]);
      smeared_hit.setTrackID(hit.getTrackID());
      smeared_hit.setPdgID(hit.getPdgID());
    }
  }// namespace sim
} //namespace tracking


DECLARE_PRODUCER_NS(tracking::sim, HitSmearingProcessor)
