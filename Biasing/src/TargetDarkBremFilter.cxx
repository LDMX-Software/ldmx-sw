/*
 * @file TargetDarkBremFilter.cxx
 * @class TargetDarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a dark brem within the target.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetDarkBremFilter.h"

SIM_PLUGIN(ldmx, TargetDarkBremFilter)

namespace ldmx { 

    std::vector<G4Track*> TargetDarkBremFilter::bremGammaTracks_ = {};

    TargetDarkBremFilter::TargetDarkBremFilter() {
        messenger_ = new TargetDarkBremFilterMessenger(this);
    }

    TargetDarkBremFilter::~TargetDarkBremFilter() {
    }

    G4ClassificationOfNewTrack TargetDarkBremFilter::stackingClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        /*std::cout << "********************************" << std::endl;*/ 
        /*std::cout << "*   Track pushed to the stack  *" << std::endl;*/
        /*std::cout << "********************************" << std::endl;*/

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        /*std::cout << "[ TargetDarkBremFilter ]: " << "\n" 
                    << "\tParticle " << particleName << " ( PDG ID: " << pdgID << " ) : " << "\n"
                    << "\tTrack ID: " << track->GetTrackID() << "\n" 
                    << std::endl;*/


        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;

        if (track->GetTrackID() == 1 && pdgID == 11) {
            ///*std::cout << "[ TargetDarkBremFilter ]: Pushing track to waiting stack." << std::endl;*/
            return fWaiting; 
        }

        return classification;
    }

    void TargetDarkBremFilter::stepping(const G4Step* step) { 

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();
        
        // Only process the primary electron track
        if (track->GetParentID() != 0) return;

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();
        
        // Make sure that the particle being processed is an electron.
        if (pdgID != 11) return; // Throw an exception

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        // Get the kinetic energy of the particle.
        //double incidentParticleEnergy = step->GetPostStepPoint()->GetTotalEnergy();
        /*std::cout << "*******************************" << std::endl;*/ 
        /*std::cout << "*   Step " << track->GetCurrentStepNumber() << std::endl;*/
        /*std::cout << "********************************" << std::endl;*/

        /*std::cout << "[ TargetDarkBremFilter ]: " << "\n" 
                    << "\tTotal energy of " << particleName      << " ( PDG ID: " << pdgID
                    << " ) : " << incidentParticleEnergy       << "\n"
                    << "\tTrack ID: " << track->GetTrackID()     << "\n" 
                    << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                    << "\tParticle currently in " << volumeName  
                    << "\tPost step process: " << step->GetPostStepPoint()->GetStepStatus() 
                    << std::endl;*/
 
        // If the particle isn't in the target, don't continue with the processing.
        if (volumeName.compareTo(volumeName_) != 0) return;

        // Check if the particle is exiting the volume.
        if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) { 
           
            // Clear all of the gamma tracks remaining from the previous event.
            TargetDarkBremFilter::bremGammaTracks_.clear();

            /*std::cout << "[ TargetDarkBremFilter ]: "
                        << "Particle " << particleName << "is leaving the "
                        << volumeName << " volume with momentum "
                        << track->GetMomentum().mag() << std::endl;*/
            
            // Get the particles daughters.
            const G4TrackVector* secondaries = step->GetSecondary();
           
            /*std::cout << "[ TargetDarkBremFilter ]: "
                        << "Incident " << particleName  << " produced " << secondaries->size() 
                        << " secondaries." << std::endl;*/
           
          
            // If the particle didn't produce any secondaries, stop processing
            // the event.
            if (secondaries->size() == 0) { 
                std::cout << "[ TargetDarkBremFilter ]: "
                            << "Primary did not produce secondaries --> Killing primary track!" 
                            << std::endl;
                
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
           
            bool hasDBrem = false;
            for (auto& secondary_track : *secondaries) {
                std::string pName = secondary_track->GetParticleDefinition()->GetParticleName();
                if (pName == "A^1") hasDBrem=true;
            }

            if (!hasDBrem) { 
                std::cout << "[ TargetDarkBremFilter ]: "
                            << "No dark brem in target --> Aborting event."
                            << std::endl;
                
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            
            }

       /*
            bool hasBremCandidate = false; 
            for (auto& secondary_track : *secondaries) {
                G4String processName = secondary_track->GetCreatorProcess()->GetProcessName();
                std::cout << "[ TargetDarkBremFilter ]: "
                            << "Secondary produced via process " << processName 
                            << std::endl;
                if (processName.compareTo("dBrem") == 0 
                        && secondary_track->GetKineticEnergy() > bremEnergyThreshold_) {
                    std::cout << "[ TargetDarkBremFilter ]: " 
                                << "Adding secondary to brem list." << std::endl;
                    TargetDarkBremFilter::bremGammaTracks_.push_back(secondary_track); 
                    hasBremCandidate = true;
                } 
            }

            if (!hasBremCandidate) { 
                std::cout << "[ TargetDarkBremFilter ]: "
                            << "The secondaries are not a result of Brem. --> Killing all tracks!"
                            << std::endl;

                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }*/

            // Check if the recoil electron should be killed.  If not, postpone 
            // its processing until the brem gamma has been processed.
            if (killRecoilElectron_) { 
                /*std::cout << "[ TargetDarkBremFilter ]: Killing electron track."
                            << std::endl;*/ 

                track->SetTrackStatus(fStopAndKill);
            } else track->SetTrackStatus(fSuspend);  

        } else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) { 
            const G4TrackVector* secondaries = step->GetSecondary();
            bool hasDBrem = false;
            for (auto& secondary_track : *secondaries) {
                std::string pName = secondary_track->GetParticleDefinition()->GetParticleName();
                if (pName =="A^1"){hasDBrem=true;}
            }
            if(!hasDBrem){
                std::cout << "[ TargetDarkBremFilter ]: "
                          << "Electron never made it out of the target --> Killing all tracks!"
                          << std::endl;

                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
        }
    }

    void TargetDarkBremFilter::endEvent(const G4Event*) {
        bremGammaTracks_.clear();
    }
    
    void TargetDarkBremFilter::removeBremFromList(G4Track* track) {   
        bremGammaTracks_.erase(std::remove(bremGammaTracks_.begin(), 
                    bremGammaTracks_.end(), track), bremGammaTracks_.end());
    }
}

