/**
 * @file TargetBremFilter.cxx
 * @class TargetBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a brem within the target.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetBremFilter.h"

namespace ldmx { 

    std::vector<G4Track*> TargetBremFilter::bremGammaTracks_ = {};

    extern "C" TargetBremFilter* createTargetBremFilter() {
        return new TargetBremFilter;
    }

    extern "C" void destroyTargetBremFilter(TargetBremFilter* object) {
        delete object;
    }

    TargetBremFilter::TargetBremFilter() {
    }

    TargetBremFilter::~TargetBremFilter() {
    }

    G4ClassificationOfNewTrack TargetBremFilter::stackingClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        /*std::cout << "********************************" << std::endl; 
        std::cout << "*   Track pushed to the stack  *" << std::endl;
        std::cout << "********************************" << std::endl;*/

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
            //std::cout << "[ TargetBremFilter ]: Pushing track to waiting stack." << std::endl;
            return fWaiting; 
        }

        return classification;
    }

    void TargetBremFilter::stepping(const G4Step* step) { 

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // Only process the primary electron track
        if (track->GetParentID() != 0) return;

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();
        
        // Make sure that the particle being processed is an electron.
        if (pdgID != 11) return; // Throw an exception

        /*std::cout << "*******************************" << std::endl;*/ 
        /*std::cout << "*   Step " << track->GetCurrentStepNumber() << std::endl;*/
        /*std::cout << "********************************" << std::endl;*/

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();
       
        // If the particle isn't in the target, don't continue with the processing.
        if (volumeName.compareTo(volumeName_) != 0) return;

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        // Get the kinetic energy of the particle.
        //double incidentParticleEnergy = step->GetPostStepPoint()->GetTotalEnergy();

        /*std::cout << "[ TargetBremFilter ]: " << "\n" 
                    << "\tTotal energy of " << particleName      << " ( PDG ID: " << pdgID
                    << " ) : " << incidentParticleEnergy       << "\n"
                    << "\tTrack ID: " << track->GetTrackID()     << "\n" 
                    << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                    << "\tParticle currently in " << volumeName  
                    << "\tPost step process: " << step->GetPostStepPoint()->GetStepStatus() 
                    << std::endl;*/

        // Check if the particle is exiting the volume.
        if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) { 
           
            // Clear all of the gamma tracks remaining from the previous event.
            TargetBremFilter::bremGammaTracks_.clear();

            /*std::cout << "[ TargetBremFilter ]: "
                      << "Particle " << particleName << "is leaving the "
                      << volumeName << " volume with momentum "
                      << track->GetMomentum().mag() << std::endl;*/
            
            if (track->GetMomentum().mag() >= recoilElectronThreshold_) { 
                /*std::cout << "[ TargetBremFilter ]: "
                          << "Electron energy is above threshold --> Aborting event."
                          << std::endl;*/
                
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            
            }

            // Get the particles daughters.
            const G4TrackVector* secondaries = step->GetSecondary();
           
            /*std::cout << "[ TargetBremFilter ]: "
                      << "Incident " << particleName  << " produced " << secondaries->size() 
                      << " secondaries." << std::endl;*/
           
          
            // If the particle didn't produce any secondaries, stop processing
            // the event.
            if (secondaries->size() == 0) { 
                /*std::cout << "[ TargetBremFilter ]: "
                          << "Primary did not produce secondaries --> Killing primary track!" 
                          << std::endl;*/
                
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
       
            bool hasBremCandidate = false; 
            for (auto& secondary_track : *secondaries) {
                G4String processName = secondary_track->GetCreatorProcess()->GetProcessName();
                /*std::cout << "[ TargetBremFilter ]: "
                          << "Secondary produced via process " << processName 
                          << std::endl;*/
                if (processName.compareTo("eBrem") == 0 
                        && secondary_track->GetKineticEnergy() > BiasingMessenger::getThreshold()) {
                    /*std::cout << "[ TargetBremFilter ]: " 
                              << "Adding secondary to brem list." << std::endl;*/
                    TargetBremFilter::bremGammaTracks_.push_back(secondary_track); 
                    hasBremCandidate = true;
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

            /*std::cout << "[ TargetBremFilter ]: "
                      << bremGammaTracks_.size() << " candidates were produced in the target --> Suspending primary track!"
                      << std::endl;*/

            // Check if the recoil electron should be killed.  If not, postpone 
            // its processing until the brem gamma has been processed.
            if (killRecoilElectron_) track->SetTrackStatus(fStopAndKill);
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

    void TargetBremFilter::endEvent(const G4Event*) {
        bremGammaTracks_.clear();
    }
    
    void TargetBremFilter::removeBremFromList(G4Track* track) {   
        bremGammaTracks_.erase(std::remove(bremGammaTracks_.begin(), 
                    bremGammaTracks_.end(), track), bremGammaTracks_.end());
    }
}

