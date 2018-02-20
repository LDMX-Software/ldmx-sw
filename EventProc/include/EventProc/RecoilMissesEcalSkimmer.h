/**
 * @file RecoilMissesEcalSkimmer.h
 * @brief Processor used to select events where the recoil electron misses the
 *        Ecal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_RECOILMISSESECALSKIMMER_H_
#define EVENTPROC_RECOILMISSESECALSKIMMER_H_

//----------//
//   LDMX   //
//----------//
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"
#include "Framework/EventProcessor.h"

namespace ldmx { 

    class RecoilMissesEcalSkimmer : public Producer { 
        
        public: 

            /** Constructor */
            RecoilMissesEcalSkimmer(const std::string &name, Process &process); 

            /** Destructor */
            ~RecoilMissesEcalSkimmer();

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param pSet Set of parameters used to configure this processor.
             */
            void configure(const ParameterSet &pSet); 

            /**
             * Run the processor and select events where the recoil misses the 
             * Ecal. 
             *
             * @param event The event to process.
             */
            void produce(Event &event); 

    }; // RecoilMissesEcalSkimmer
}

#endif // EVENTPROC_RECOILMISSESECALSKIMMER_H_
