/**
 * @file FindableTrackProcessor.cxx
 * @brief Processor used to find all particles that pass through the recoil
 *        tracker and leave hits consistent with a findable track.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/FindableTrackProcessor.h"

namespace ldmx { 

    FindableTrackProcessor::FindableTrackProcessor(const std::string &name, const Process &process) :
        EventProcessor(name, process) { 
    }

    FindableTrackProcessor::~FindableTrackProcessor() { 
    }

    void FindableTrackProcessor::produce(Event &event) { 
       
        const TClonesArray *simParticles = event.getCollection("SimParticles"); 
        if (simParticles->GetEntriesFast() == 0) return; 
    
        SimParticle* recoilElectron = nullptr; 

        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); ++particleCount) { 
            SimParticle* simParticle = static_cast<SimParticle*>(simParticles->At(particleCount));
            
            if (simParticle->getPdgID() == 11 && simParticle->getGenStatus() == 1) { 
                recoilElectron == simParticle;  
            }
        }

        // Get the collection of Recoil sim hits from the event
        const TClonesArray *recoilSimHits = event.getCollection("RecoilSimHits");  

        std::vector<int> recoilHitCount(10, 0);
        for (int hitCount = 0; hitCount < recoilSimHits->GetEntriesFast(); ++hitCount) { 
            SimTrackerHit* recoilSimHit = static_cast<SimTrackerHit*>(recoilSimHits->At(hitCount));
            if (recoilSimHit->getSimParticle() == recoilElectron) { 
                recoilHitCount[recoilSimHit->getLayerID()]++;  
            }
        }
    }
}
