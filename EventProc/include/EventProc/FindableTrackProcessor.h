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
#include "Event/Event.h"
#include "Event/FindableTrackResult.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "Framework/EventProcessor.h"

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>

namespace ldmx { 

    class FindableTrackProcessor : public Producer { 
        
        public: 

            /** Constructor */
            FindableTrackProcessor(const std::string &name, const Process &process); 

            /** Destructor */
            ~FindableTrackProcessor();

            /** 
             *
             */
            void configure(const ParameterSet &pset); 

            /**
             *
             */
            void produce(Event &event); 
            
        private:

            /**
             *
             */
            void createHitMap(const TClonesArray* recoilSimHits);
          
            /** */
            bool isFindable(std::vector<int> hitCount); 

            /** */ 
            std::map<SimParticle*, std::vector<int>> hitMap_;

            /** */
            TClonesArray* findableTrackResults_{nullptr};


    }; // FindableTrackProcessor
}

#endif // EVENTPROC_FINDABLETRACKPROCESSOR_H_

