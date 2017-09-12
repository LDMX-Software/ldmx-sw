/**
 * @file HcalStripVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/HcalStripVetoProcessor.h"

namespace ldmx {

    HcalStripVetoProcessor::HcalStripVetoProcessor(const std::string &name, Process &process) : 
            Producer(name, process) {
    }
    
    HcalStripVetoProcessor::~HcalStripVetoProcessor() { 
    }

    void HcalStripVetoProcessor::configure(const ParameterSet& pSet) {
        totalPEThreshold_  = pSet.getDouble("pe_threshold"); 
    }

    void HcalStripVetoProcessor::produce(Event& event) {

        // Get the collection of sim particles from the event 
        const TClonesArray *hcalStripHits = event.getCollection("hcalStripDigis");
        //if (hcalHits->GetEntriesFast() == 0) return; 
       
        // Loop over all of the Hcal hits and calculate to total photoelectrons
        // in the event.
        float totalPe{0}, maxPE{0}; 
        for (int iHit = 0; iHit < hcalStripHits->GetEntriesFast(); ++iHit) { 
            HcalStripHit* hcalStripHit = (HcalStripHit*) hcalStripHits->At(iHit);
            //std::cout << "[ HcalVeto ]: Hit PE: " << hcalHit->getPE() << std::endl;
            totalPe += hcalStripHit->getPE(); 
            maxPE = std::max(maxPE,hcalStripHit->getPE());
        }

        bool passesVeto{true}; 
        //std::cout << "[ HcalVeto ]: total PE: " << totalPe << std::endl;
        if (totalPe >= totalPEThreshold_) passesVeto = false;
        //if (maxPE >= totalPEThreshold_) passesVeto = false;
        
        result_.setResult(passesVeto); 
        event.addToCollection("HcalStripVeto", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, HcalStripVetoProcessor);
