/**
 * @file HcalStripVetoProcessor.h
 * @brief Processor that determines if an event is vetoed by the Hcal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_HCALSTRIPVETOPROCESSOR_H_
#define EVENTPROC_HCALSTRIPVETOPROCESSOR_H_

//----------//
//   LDMX   //
//----------//
#include "Event/HcalVetoResult.h"
#include "Event/HcalStripHit.h"
#include "Framework/EventProcessor.h"

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>

namespace ldmx { 

    class HcalStripVetoProcessor : public Producer { 
        
        public: 

            /** Constructor */
            HcalStripVetoProcessor(const std::string &name, Process &process); 

            /** Destructor */
            ~HcalStripVetoProcessor();

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

    }; // HcalStripVetoProcessor
}

#endif // EVENTPROC_HcalStripVetoProcessor_H_

