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
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "Framework/EventProcessor.h"

namespace ldmx { 

    class FindableTrackProcessor : public EventProcessor { 
        
        public: 

            /** Constructor */
            FindableTrackProcessor(const std::string &name, const Process &process); 

            /** Destructor */
            ~FindableTrackProcessor();

            /**
             *
             */
            void produce(Event &event); 
            
        private:

    }; // FindableTrackProcessor
}

#endif // EVENTPROC_FINDABLETRACKPROCESSOR_H_

