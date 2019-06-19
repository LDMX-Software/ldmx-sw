/**
 * @file HcalVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/HcalVetoProcessor.h"

<<<<<<< HEAD
//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "DetDescr/HcalID.h"

=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
namespace ldmx {

    HcalVetoProcessor::HcalVetoProcessor(const std::string &name, Process &process) : 
            Producer(name, process) {
    }
    
    HcalVetoProcessor::~HcalVetoProcessor() { 
    }

    void HcalVetoProcessor::configure(const ParameterSet& pSet) {
<<<<<<< HEAD
        totalPEThreshold_  = pSet.getDouble("pe_threshold");
        maxTime_ = pSet.getDouble("max_time"); 
        maxDepth_ = pSet.getDouble("max_depth"); 
        minPE_ = pSet.getDouble("back_min_pe");  
=======
        totalPEThreshold_  = pSet.getDouble("pe_threshold"); 
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    }

    void HcalVetoProcessor::produce(Event& event) {

        // Get the collection of sim particles from the event 
        const TClonesArray *hcalHits = event.getCollection("hcalDigis");
       
        // Loop over all of the Hcal hits and calculate to total photoelectrons
        // in the event.
        float totalPe{0};
<<<<<<< HEAD
        float maxPE{-1000};
        HcalHit* maxPEHit{nullptr}; 
        for (size_t iHit{0}; iHit < hcalHits->GetEntriesFast(); ++iHit) { 
            HcalHit* hcalHit = static_cast<HcalHit*>(hcalHits->At(iHit));

            // If the hit time is outside the readout window, don't consider it.
            if (hcalHit->getTime() >= maxTime_) continue;

            // If the hit z position is beyond the maximum HCal depth, skip it.
            if (hcalHit->getZ() > maxDepth_) continue;

            // Get the total PE in the bar
            float pe = hcalHit->getPE(); 

            // Keep track of the total PE
            totalPe += pe; 

            // Check that both sides of the bar have a PE value above threshold.
            // If not, don't consider the hit.  Double sided readout is only 
            // being used for the back HCal bars.  For the side HCal, just 
            // use the maximum PE as before.
            if ( (hcalHit->getSection() == HcalSection::BACK) && (hcalHit->getMinPE() < minPE_) ) continue;

            // Find the maximum PE in the list
            if (maxPE < pe) {
                maxPE = pe;
                maxPEHit = hcalHit; 
            }
        }

        // If the maximum PE found is below threshold, it passes the veto.
        bool passesVeto = (maxPE < totalPEThreshold_) ? true : false;

        HcalVetoResult result; 
        result.setVetoResult(passesVeto);
        result.setMaxPEHit(maxPEHit); 
=======
        float maxPE{0}; 
        for (int iHit = 0; iHit < hcalHits->GetEntriesFast(); ++iHit) { 
            HcalHit* hcalHit = (HcalHit*) hcalHits->At(iHit);
            //std::cout << "[ HcalVeto ]: Hit PE: " << hcalHit->getPE() << std::endl;
            totalPe += hcalHit->getPE(); 
            maxPE = std::max(maxPE,hcalHit->getPE());
        }

        bool passesVeto{true}; 
        //std::cout << "[ HcalVeto ]: total PE: " << totalPe << std::endl;
        if (maxPE >= totalPEThreshold_) passesVeto = false;
        result_.setResult(passesVeto); 
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

        if (passesVeto) { 
            setStorageHint(hint_shouldKeep); 
        } else { 
            setStorageHint(hint_shouldDrop); 
        } 

<<<<<<< HEAD
        event.addToCollection("HcalVeto", result);
=======
        event.addToCollection("HcalVeto", result_);
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    }
}

DECLARE_PRODUCER_NS(ldmx, HcalVetoProcessor);
