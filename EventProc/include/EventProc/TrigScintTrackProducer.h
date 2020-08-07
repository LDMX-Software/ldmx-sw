/**
 * @file TrigScintTrackProducer.h
 * @brief
 * @author
 */

#ifndef EVENTPROC_TRIGSCINTTRACKPRODUCER_H
#define EVENTPROC_TRIGSCINTTRACKPRODUCER_H

//LDMX Framework
#include "Framework/Event.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/Parameters.h" // Needed to import parameters from configuration file
#include "Event/TrigScintTrack.h"
#include "Event/EventConstants.h"
#include "Event/TrigScintCluster.h"


#include "TClonesArray.h"

namespace ldmx {
    
    /**
     * @class TrigScintTrackProducer
     * @brief making tracks from trigger scintillator clusters 
     */
    class TrigScintTrackProducer : public ldmx::Producer {
        public:

            TrigScintTrackProducer(const std::string& name, ldmx::Process& process) : ldmx::Producer(name, process) {
	//	    	tracks_ = new TClonesArray("ldmx::TrigScintTrack");
	    }

      //virtual void configure(const ldmx::ParameterSet& ps);
      virtual void configure(ldmx::Parameters& ps);
      
      virtual void produce(ldmx::Event& event);
      
      virtual void onFileOpen();
      
      virtual void onFileClose();
      
      virtual void onProcessStart(); 
      
      virtual void onProcessEnd();

        private:
      std::vector< TrigScintTrack > tracks_;
                                                                   
      TrigScintTrack makeTrack( std::vector<TrigScintCluster> clusters );
      
      double maxDelta_{0.};
      int verbose_{0};

      std::string seeding_collection_;
       std::vector <std::string> input_collections_;
       //std::string input_collection1_;
       //std::string input_collection2_;
      std::string output_collection_;
      std::string passName_{""};

      float centroid_{0.};  //channel nb centroid (will not be content weighted)
      float residual_{0.};  //channel nb residual (will not be content weighted)


    };
}

#endif /* EVENTPROC_TRIGSCINTTRACKPRODUCER_H */
