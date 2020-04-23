/*
 * @file TargetBremFilter.cxx
 * @class TargetBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a brem within the target.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetBremFilter.h"

namespace ldmx { 

    std::vector<G4Track*> TargetBremFilter::bremGammaTracks_ = {};

    TargetBremFilter::TargetBremFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) { 
        
        recoilMaxPThreshold_     = parameters.getParameter< double >("recoil_max_p_threshold"); 
        bremEnergyThreshold_     = parameters.getParameter< double >("brem_min_energy_threshold"); 
        killRecoil_              = parameters.getParameter<bool>("kill_recoil_track"); 
    }

    TargetBremFilter::~TargetBremFilter() {
    }

    G4ClassificationOfNewTrack TargetBremFilter::ClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        /*std::cout << "********************************" << std::endl;*/ 
        /*std::cout << "*   Track pushed to the stack  *" << std::endl;*/
        /*std::cout << "********************************" << std::endl;*/

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        /*std::cout << "[ TargetBremFilter ]: " << "\n" 
                    << "\tParticle " << particleName << " ( PDG ID: " << pdgID << " ) : " << "\n"
                    << "\tTrack ID: " << track->GetTrackID() << "\n" 
                    << std::endl;*/


        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;

        if (track->GetTrackID() == 1 && pdgID == 11) {
            ///*std::cout << "[ TargetBremFilter ]: Pushing track to waiting stack." << std::endl;*/
            return fWaiting; 
        }

        return classification;
    }

    void TargetBremFilter::stepping(const G4Step* step) { 

        // Get the track associated with this step.
        auto track{step->GetTrack()};

        // Only process the primary electron track
        if (track->GetParentID() != 0) return;

        // Get the PDG ID of the track and make sure it's an electron. If 
        // another particle type is found, thrown an exception. 
        if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11) return; 

        // Get the region the particle is currently in.  Continue processing
        // the particle only if it's in the target region. 
        if (auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
                region.compareTo("target") != 0) return;


        // Check if the electron will be exiting the target        
        if (auto volume{track->GetNextVolume()->GetName()}; volume.compareTo("recoil_PV") == 0) {
        
            // Clear all of the gamma tracks remaining from the previous event.
            TargetBremFilter::bremGammaTracks_.clear();

            /*std::cout << "[ TargetBremFilter ]: Electron is leaving "
                        << track->GetVolume()->GetName() << " with momentum "
                        << track->GetMomentum().mag() << std::endl;*/
            
            // If the recoil electron  
            if (track->GetMomentum().mag() >= recoilMaxPThreshold_) { 
               /* std::cout << "[ TargetBremFilter ]: "
                            << "Electron energy is above threshold --> Aborting event."
                            << std::endl;*/
                
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            
            }

            // Get the electron secondries
            bool hasBremCandidate = false; 
            if (auto secondaries = step->GetSecondary(); secondaries->size() == 0) {
                /*std::cout << "[ TargetBremFilter ]: "
                            << "Primary did not produce secondaries --> Killing primary track!" 
                            << std::endl;*/
                
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            } else {
                /*std::cout << "[ TargetBremFilter ]: "
                            << "Incident " << particleName  << " produced " << secondaries->size() 
                            << " secondaries." << std::endl;*/
                // TODO: Instead of pushing a track to a list, tag it. 
                for (auto& secondary_track : *secondaries) {
                    G4String processName = secondary_track->GetCreatorProcess()->GetProcessName();
                    /*std::cout << "[ TargetBremFilter ]: "
                                << "Secondary produced via process " << processName 
                                << std::endl;*/
                    if (processName.compareTo("eBrem") == 0 
                            && secondary_track->GetKineticEnergy() > bremEnergyThreshold_) {
                        //std::cout << "[ TargetBremFilter ]: " 
                        //          << "Adding secondary to brem list." << std::endl;
                        TargetBremFilter::bremGammaTracks_.push_back(secondary_track); 
                        hasBremCandidate = true;
                    } 
                }
            }

            if (!hasBremCandidate) { 
                /*std::cout << "[ TargetBremFilter ]: "
                            << "The secondaries are not a result of Brem. --> Killing all tracks!"
                            << std::endl;*/

                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }

            // Check if the recoil electron should be killed.  If not, postpone 
            // its processing until the brem gamma has been processed.
            if (killRecoil_) track->SetTrackStatus(fStopAndKill);
            else track->SetTrackStatus(fSuspend);  

        } else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) { 
            /*std::cout << "[ TargetBremFilter ]: "
                        << "Electron never made it out of the target --> Killing all tracks!"
                        << std::endl;*/

            track->SetTrackStatus(fKillTrackAndSecondaries);
            G4RunManager::GetRunManager()->AbortEvent();
            return;
        }
    }

    void TargetBremFilter::EndOfEventAction(const G4Event*) {
        bremGammaTracks_.clear();
    }
    
    void TargetBremFilter::removeBremFromList(G4Track* track) {   
        bremGammaTracks_.erase(std::remove(bremGammaTracks_.begin(), 
                    bremGammaTracks_.end(), track), bremGammaTracks_.end());
    }
}

DECLARE_ACTION(ldmx, TargetBremFilter) 
