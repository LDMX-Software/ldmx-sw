/**
 * @file TrackerVetoProcessor.h
 * @brief Processor that determines if an event is vetoed by the Recoil tracker. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __EVENTPROC_TRACKER_VETO_PROCESSOR_H__
#define __EVENTPROC_TRACKER_VETO_PROCESSOR_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <string>

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace ldmx { 

    class TrackerVetoProcessor : public Producer { 
        
        public: 

            /** Constructor */
            TrackerVetoProcessor(const std::string &name, Process &process); 

            /** Destructor */
            ~TrackerVetoProcessor();

            /**
             * Run the processor and create a collection of results which 
             * indicate if the event passes/fails the Hcal veto.
             *
             * @param event The event to process.
             */
            void produce(Event &event);
 
        private:

    }; // TrackerVetoProcessor
}

#endif // __EVENTPROC_TRACKER_VETO_PROCESSOR_H__

