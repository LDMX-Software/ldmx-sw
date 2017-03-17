/**
 * @file HcalVetoProcessor.h
 * @brief Processor that determines if an event is vetoed by the Hcal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_HCALVETOPROCESSOR_H_
#define EVENTPROC_HCALVETOPROCESSOR_H_

//----------//
//   LDMX   //
//----------//
#include "Event/Event.h"
#include "Event/HcalVetoResult.h"
#include "Event/HcalHit.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>

namespace ldmx { 

    class HcalVetoProcessor : public Producer { 
        
        public: 

            /** Constructor */
            HcalVetoProcessor(const std::string &name, Process &process); 

            /** Destructor */
            ~HcalVetoProcessor();

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param pSet Set of parameters used to configure this processor.
             */
            void configure(const ParameterSet &pSet); 

            /**
             * Run the processor and create a collection of results which 
             * indicate if the event passes/fails the Hcal veto.
             *
             * @param event The event to process.
             */
            void produce(Event &event); 
            
        private:

            /** Collection of results. */
            HcalVetoResult result_;

            /** Total PE threshold. */
            double totalPEThreshold_{8};

    }; // HcalVetoProcessor
}

#endif // EVENTPROC_HCALVETOPROCESSOR_H_

