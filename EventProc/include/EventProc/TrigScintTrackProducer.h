/**
 * @file TrigScintTrackProducer.h
 * @brief making tracks from trigger scintillator clusters 
 * @author Lene Kristian Bryngemark, Stanford University
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


namespace ldmx {
    
    /**
     * @class TrigScintTrackProducer
     * @brief making tracks from trigger scintillator clusters 
     */
    class TrigScintTrackProducer : public ldmx::Producer {
        public:

            TrigScintTrackProducer(const std::string& name, ldmx::Process& process) : ldmx::Producer(name, process) {
	    }

      virtual void configure(ldmx::Parameters& ps);
      
      virtual void produce(ldmx::Event& event);
      
      virtual void onFileOpen();
      
      virtual void onFileClose();
      
      virtual void onProcessStart(); 
      
      virtual void onProcessEnd();

        private:

	  //collection of produced tracks
      std::vector< TrigScintTrack > tracks_;
	  
	  //add a cluster to a track
	  TrigScintTrack makeTrack( std::vector<TrigScintCluster> clusters );

	  //maximum difference (in channel number space) between track seed and cluster in the next pad tolerated to form a track
      double maxDelta_{0.};
	  
	  //producer specific verbosity 
      int verbose_{0};
	  
	  //collection used to seed the tracks 
      std::string seeding_collection_;
	  
	  //other cluster collections used in track making
	  std::vector <std::string> input_collections_;
	  
	  //output collection (tracks)
	  std::string output_collection_;
	  
	  //specific pass name to use for track making 
	  std::string passName_{""};
	  
	  //track centroid in units of channel nb (will not be content weighted)
      float centroid_{0.};
	  
	  //track residual in units of channel nb (will not be content weighted)
      float residual_{0.};  
	  
    };
}

#endif /* EVENTPROC_TRIGSCINTTRACKPRODUCER_H */
