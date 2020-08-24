/**
 * @file FindableTrackProcessor.h
 * @brief Processor used to find all particles that pass through the recoil
 *        tracker and leave hits consistent with a findable track.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_FINDABLETRACKPROCESSOR_H_
#define EVENTPROC_FINDABLETRACKPROCESSOR_H_

//----------//
//   LDMX   //
//----------//
#include "Event/SiStripHit.h"
#include "Event/FindableTrackResult.h"
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

namespace ldmx { 

    class FindableTrackProcessor : public Producer { 
        
        public: 

            /** Constructor */
            FindableTrackProcessor(const std::string &name, Process &process); 

            /** Destructor */
            ~FindableTrackProcessor();

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this processor.
             */
            void configure(Parameters& parameters) final override; 

            /**
             * Run the processor and create a collection of results which 
             * indicate if a charge particle can be found by the recoil tracker.
             *
             * @param event The event to process.
             */
            void produce(Event &event); 
            
        private:

            /**
             * Create a hit map which associates a charged particle to it's 
             * hits in the recoil tracker.
             *
             * @param recoilSimHit collection of simulated recoil tracker hits.
             */
            void createHitMap(const std::vector<SiStripHit> &siStripHits);
          
            /** 
             * Given a set of hits, check if a sim particle is expected to fall
             * within the acceptance of the recoil tracker.
             *
             * @param result The object used to encapsulate the results.
             * @param hitCount Vector indicating whether the nth layer of the 
             *                 recoil tracker has a hit.
             */
            void isFindable(FindableTrackResult &result, std::vector<int> hitCount); 

            /** Map between a sim particle and it's hit array. */ 
            std::map<int, std::vector<int>> hitMap_;

    }; // FindableTrackProcessor
}

#endif // EVENTPROC_FINDABLETRACKPROCESSOR_H_

