/**
 * @file OverlayProducer.h
 * @brief Class to overlay in-time pile-up events from an overlay file 
 * @author Lene Kristian Bryngemark, Stanford University 
 */

#ifndef EVENTPROC_OVERLAYPRODUCER_H
#define EVENTPROC_OVERLAYPRODUCER_H

//ROOT
#include "TFile.h"
#include "TRandom2.h"

// STL
#include <string>
#include <vector>
#include <map>


//LDMX Framework
#include "Event/EventDef.h" // Includes all possible event bus objects
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h" // Needed to declare processor
#include "Framework/Parameters.h" // Needed to import parameters from configuration file


namespace ldmx {
    
    /**
     * @class OverlayProducer
     * @brief 
     */
  class OverlayProducer : public ldmx::Producer {
  public:

		OverlayProducer(const std::string& name, ldmx::Process& process) : ldmx::Producer(name, process) {  }

		  ~OverlayProducer();

	  virtual void configure(Parameters& parameters) final override;

            virtual void produce(Event& event) final override;

	virtual void onFileOpen();

            virtual void onFileClose(EventFile&);

            virtual void onProcessStart(); 

            virtual void onProcessEnd();

        private:

	    /**
	     * overlay events input file
	     */

	    std::unique_ptr<EventFile> overlayFile_;      
	    
            /**
             * The overlay ldmx event bus
             */
            Event overlayEvent_;

	    std::unique_ptr<TTree> overlayTree_;      // overlay events input tree
	    std::string overlayFileName_;   // overlay events input file name
	    std::string overlayTreeName_;   // overlay events input tre name
	    std::string overlayPassName_;   // overlay events input file name
	    //	    std::unique_ptr<EventFile> simFile_;      // sim file: the file being generated, to be overlaid 
	    EventFile *simFile_;      // sim file: the file being generated, to be overlaid 
	    int lastEvent_{0};         // book keeping of used events
	    int doPoisson_{0};         // or not
	    double poissonMu_{0.};       // av. nOverlay from the given process
	    int nEventsTot_{0};        // in the input tree 
	    std::vector <std::string> collections_;     // to loop over and add hits from
	    std::string overlayProcessName_;  // to use for naming the collection where nOverlaidEvents is relayed
	    std::string passName_;          // to use for naming the event bus passengers, mostly a disambiguation
	    std::string simParticleCollName_;          // to use for naming the event bus passengers, mostly a disambiguation
	    std::string simParticlePassName_;          // to use for naming the event bus passengers, mostly a disambiguation
	    int verbosity_;            // control verbosity 
	    std::unique_ptr<TRandom2> rndm_;  // random number generator. TRandom2 slightly (~10%) faster than TRandom3; shorter period but our input files will have way shorter period anyway. 
	    //	    TRandom2 * rndm_{NULL};     
	    int seed_{0};              // random number generator seed. not clear if this needs to be reproducibly independent or can just be clock seeded (using 0). 
    };
}

#endif /* EVENTPROC_OVERLAYPRODUCER_H */
