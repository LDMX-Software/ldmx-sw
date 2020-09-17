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
#include "Tools/AnalysisUtils.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimParticle.h"
#include "Framework/EventProcessor.h"

namespace ldmx { 

    class RecoilMissesEcalSkimmer : public Producer { 
        
        public: 

            /** Constructor */
            RecoilMissesEcalSkimmer(const std::string &name, Process &process); 

            /** Destructor */
            ~RecoilMissesEcalSkimmer();

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
