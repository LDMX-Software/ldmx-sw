/**
 * @file HcalVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/HcalVetoProcessor.h"

namespace ldmx {

    HcalVetoProcessor::HcalVetoProcessor(const std::string &name, Process &process) : 
        Producer(name, process) { 
    }
    
    HcalVetoProcessor::~HcalVetoProcessor() { 
    }

    void HcalVetoProcessor::configure(const ParameterSet& pSet) {
        totalPEThreshold_  = pSet.getDouble("pe_threshold"); 
    }

    void HcalVetoProcessor::produce(Event& event) {

        // Get the collection of sim particles from the event 
        const TClonesArray *hcalHits = event.getCollection("hcalDigis");
        //if (hcalHits->GetEntriesFast() == 0) return; 
       
        // Loop over all of the Hcal hits and calculate to total photoelectrons
        // in the event.
        double totalPe{0}; 
        for (int iHit = 0; iHit < hcalHits->GetEntriesFast(); ++iHit) { 
            HcalHit* hcalHit = (HcalHit*) hcalHits->At(iHit);
            //std::cout << "[ HcalVeto ]: Hit PE: " << hcalHit->getPE() << std::endl;
            totalPe += hcalHit->getPE(); 
        }

        bool passesVeto{true}; 
        //std::cout << "[ HcalVeto ]: total PE: " << totalPe << std::endl;
        if (totalPe >= totalPEThreshold_) passesVeto = false;
        
        result_.setResult(passesVeto); 
        event.addToCollection("HcalVeto", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, HcalVetoProcessor);
