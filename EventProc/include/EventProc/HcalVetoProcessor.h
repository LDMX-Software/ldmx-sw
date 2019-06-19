/**
 * @file HcalVetoProcessor.h
 * @brief Processor that determines if an event is vetoed by the Hcal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

<<<<<<< HEAD
#ifndef __EVENTPROC_HCAL_VETO_PROCESSOR_H__
#define __EVENTPROC_HCAL_VETO_PROCESSOR_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <string>
=======
#ifndef EVENTPROC_HCALVETOPROCESSOR_H_
#define EVENTPROC_HCALVETOPROCESSOR_H_
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

//----------//
//   LDMX   //
//----------//
#include "Event/HcalVetoResult.h"
#include "Event/HcalHit.h"
#include "Framework/EventProcessor.h"

<<<<<<< HEAD
=======
//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>

>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
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
<<<<<<< HEAD
            void produce(Event &event);
 
        private:

            /** Total PE threshold. */
            double totalPEThreshold_{8};

            /** Maximum hit time that should be considered by the veto. */
            float maxTime_{50}; // ns

            /** Maximum z depth that a hit can have. */
            float maxDepth_{4000}; // mm 

            /** The minimum number of PE needed for a hit. */
            float minPE_{1}; 

=======
            void produce(Event &event); 
            
        private:

            /** Collection of results. */
            HcalVetoResult result_;

            /** Total PE threshold. */
            double totalPEThreshold_{8};

>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    }; // HcalVetoProcessor
}

#endif // EVENTPROC_HCALVETOPROCESSOR_H_

