/**
 * @file HcalVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/HcalVetoProcessor.h"

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

namespace ldmx {

    HcalVetoProcessor::HcalVetoProcessor(const std::string &name, Process &process) : 
            Producer(name, process) {
    }
    
    HcalVetoProcessor::~HcalVetoProcessor() { 
    }

    void HcalVetoProcessor::configure(const ParameterSet& pSet) {
        totalPEThreshold_  = pSet.getDouble("pe_threshold");
        maxTime_ = pSet.getDouble("max_time");  
    }

    void HcalVetoProcessor::produce(Event& event) {

        // Get the collection of sim particles from the event 
        const TClonesArray *hcalHits = event.getCollection("hcalDigis");
       
        // Loop over all of the Hcal hits and calculate to total photoelectrons
        // in the event.
        float totalPe{0};
        float maxPE{-1000};
        HcalHit* maxPEHit{nullptr}; 
        for (size_t iHit{0}; iHit < hcalHits->GetEntriesFast(); ++iHit) { 
            HcalHit* hcalHit = static_cast<HcalHit*>(hcalHits->At(iHit));

            // If the hit time is outside the readout window, don't consider it.
            if (hcalHit->getTime() >= maxTime_) continue;

            float pe = hcalHit->getPE();
            
            // Keep track of the total PE
            totalPe += pe; 
            
            // Find the maximum PE in the list
            if (maxPE < pe) {
                maxPE = pe;
                maxPEHit = hcalHit; 
            }
        }

        // If the maximum PE found is below threshold, it passes the veto.
        bool passesVeto{false}; 
        if (maxPE < totalPEThreshold_) passesVeto = true;

        HcalVetoResult result; 
        result.setVetoResult(passesVeto);
        result.setMaxPEHit(maxPEHit); 

        if (passesVeto) { 
            setStorageHint(hint_shouldKeep); 
        } else { 
            setStorageHint(hint_shouldDrop); 
        } 

        event.addToCollection("HcalVeto", result);
    }
}

DECLARE_PRODUCER_NS(ldmx, HcalVetoProcessor);
