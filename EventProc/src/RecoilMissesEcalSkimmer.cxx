/**
 * @file RecoilMissesEcalSkimmer.cxx
 * @brief Processor used to select events where the recoil electron misses the
 *        Ecal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/RecoilMissesEcalSkimmer.h"

namespace ldmx { 
    
    RecoilMissesEcalSkimmer::RecoilMissesEcalSkimmer(const std::string &name, Process &process):
        Producer(name, process) { 
    }

    RecoilMissesEcalSkimmer::~RecoilMissesEcalSkimmer() { 
    }

    void RecoilMissesEcalSkimmer::configure(const ParameterSet &pset) { 
    }

    void RecoilMissesEcalSkimmer::produce(Event &event) { 
        
        // Get the collection of sim particles from the event 
        const TClonesArray *simParticles = event.getObject<TClonesArray *>("SimParticles");
        if (simParticles->GetEntriesFast() == 0) return; 

        // Loop through all of the particles and search for the recoil electron
        // i.e. an electron which doesn't have any parents.
        SimParticle* recoilElectron{nullptr};
        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); ++particleCount) { 
            
            // Get the nth particle from the collection of particles
            SimParticle* simParticle = static_cast<SimParticle*>(simParticles->At(particleCount));

            // If the particle doesn't correspond to the recoil electron, 
            // continue to the next particle.
            if ((simParticle->getPdgID() == 11) && (simParticle->getParentCount() == 0)) {
                //std::cout << "[ pnWeightProcessor ]: Recoil electron found." << std::endl;
                recoilElectron = simParticle; 
                break;
            }
        }

        // Get the collection of simulated Ecal hits from the event. 
        const TClonesArray* ecalSimHits = event.getObject<TClonesArray *>(EventConstants::ECAL_SIM_HITS);
       
        // Loop through the Ecal hits and check if the recoil electron is 
        // associated with any of them.  If there are any recoil electron hits
        // in the Ecal, drop the event.
        bool hasRecoilElectronHits = false; 
        for (int iHit = 0; iHit < ecalSimHits->GetEntriesFast(); ++iHit) { 
            
            SimCalorimeterHit* simHit = static_cast<SimCalorimeterHit*>(ecalSimHits->At(iHit));

            /*std::cout << "[ RecoilMissesEcalSkimmer ]: "  
                      << "Number of hit contributions: "  
                      << simHit->getNumberOfContribs() << std::endl;*/

            for (int iContrib = 0; iContrib < simHit->getNumberOfContribs(); ++iContrib) {
                SimCalorimeterHit::Contrib contrib = simHit->getContrib(iContrib);

                if (contrib.trackID == recoilElectron->getTrackID()) { 
                    /*std::cout << "[ RecoilMissesEcalSkimmer ]: " 
                              << "Ecal hit associated with recoil electron." << std::endl; */
                    
                    hasRecoilElectronHits = true;
                } 
            } 
        }
       
        // Tell the skimmer to keep or drop the event based on whether there
        // were recoil electron hits found in the Ecal. 
        if (hasRecoilElectronHits) { 
            setStorageHint(hint_shouldDrop); 
        } else { 
            setStorageHint(hint_shouldKeep); 
        }
    }
}

DECLARE_PRODUCER_NS(ldmx, RecoilMissesEcalSkimmer);
