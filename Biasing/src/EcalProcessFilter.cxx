/**
 * @file EcalProcessFilter.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "Biasing/EcalProcessFilter.h"

namespace ldmx { 

    extern "C" EcalProcessFilter* createEcalProcessFilter() {
        return new EcalProcessFilter;
    }

    extern "C" void destroyEcalProcessFilter(EcalProcessFilter* object) {
        delete object;
    }


    EcalProcessFilter::EcalProcessFilter() { 
    }

    EcalProcessFilter::~EcalProcessFilter() { 
    }

    G4ClassificationOfNewTrack EcalProcessFilter::stackingClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        /*std::cout << "********************************" << std::endl; 
        std::cout << "*   Track pushed to the stack  *" << std::endl;
        std::cout << "********************************" << std::endl;*/

        // get the PDGID of the track.
        //G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        /*std::cout << "[ TargetBremFilter ]: " << "\n" 
                    << "\tParticle " << particleName << " ( PDG ID: " << pdgID << " ) : " << "\n"
                    << "\tTrack ID: " << track->GetTrackID() << "\n" 
                    << std::endl;*/

        if (track == currentTrack_) {
            currentTrack_ = nullptr; 
            //std::cout << "[ TargetBremFilter ]: Pushing track to waiting stack." << std::endl;
            return fWaiting; 
        }

        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;
        
        return classification;
    }

    void EcalProcessFilter::stepping(const G4Step* step) { 

        if (TargetBremFilter::getBremGammaList().empty()) { 
            return;
        } 
        
        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // Only process tracks whose parent is the primary particle
        if (track->GetParentID() != 1) return; 

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Make sure that the particle being processed is an electron.
        if (pdgID != 22) return; // Throw an exception

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();

        // If the particle isn't in the Ecal, don't continue with the processing.
        std::vector<G4Track*> bremGammaList = TargetBremFilter::getBremGammaList();
        if ((!volumeName.contains("W") && !volumeName.contains("Si"))
                || !volumeName.contains("phys")) {
            if (step->GetSecondary()->size() != 0 
                    && (std::find(bremGammaList.begin(), bremGammaList.end(), track) != bremGammaList.end())) { 
                
                /*std::cout << "[ EcalProcessFilter ]: "
                          << "Reaction occured outside volume of intereset --> Aborting event." 
                          << std::endl;*/
                if (bremGammaList.size() == 1) { 
                    track->SetTrackStatus(fKillTrackAndSecondaries);
                    G4RunManager::GetRunManager()->AbortEvent();
                    currentTrack_ = nullptr;
                    return;
                } else {
                    currentTrack_ = track; 
                    track->SetTrackStatus(fSuspend);
                    TargetBremFilter::removeBremFromList(track);
                    return;
                }
            }
            return;
        }

        /*std::cout << "*******************************" << std::endl; 
        std::cout << "*   Step " << track->GetCurrentStepNumber() << std::endl;
        std::cout << "********************************" << std::endl;*/
        
        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();
        
        // Get the kinetic energy of the particle.
        //double incidentParticleEnergy = step->GetPreStepPoint()->GetTotalEnergy();

        /*std::cout << "[ EcalProcessFilter ]:\n" 
                    << "\tTotal energy of " << particleName  << ": " << incidentParticleEnergy << " MeV \n"
                    << "\tPDG ID: " << pdgID << "\n"
                    << "\tTrack ID: " << track->GetTrackID() << "\n" 
                    << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                    << "\tParent ID: " << track->GetParentID() << "\n"
                    << "\tParticle currently in " << volumeName  << std::endl;*/

        // 
        if (std::find(bremGammaList.begin(), bremGammaList.end(), track) == bremGammaList.end()) { 
            /*std::cout << "[ EcalProcessFilter ]: "
                      << "Brem list doesn't contain track." << std::endl;*/
            currentTrack_ = track; 
            track->SetTrackStatus(fSuspend);
            return;
        }
 
        // Get the particles daughters.
        const G4TrackVector* secondaries = step->GetSecondary();

        if (secondaries->size() == 0) { 
            
            /*std::cout << "[ EcalProcessFilter ]: "
                      << "Brem photon did not interact --> Continue propogating track."
                      << std::endl;*/    
        } else { 
            G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
            
            /*std::cout << "[ EcalProcessFilter ]: "
                      << "Brem photon produced " << secondaries->size() 
                      << " particle via " << processName << " process." 
                      << std::endl;*/

            // Only record the process that is being biased
            if (!processName.contains(BiasingMessenger::getProcess())) {

                /*std::cout << "[ EcalProcessFilter ]: "
                          << "Process was not " << BiasingMessenger::getProcess() 
                          << std::endl;*/
                
                if (bremGammaList.size() == 1) { 
                    track->SetTrackStatus(fKillTrackAndSecondaries);
                    G4RunManager::GetRunManager()->AbortEvent();
                    currentTrack_ = nullptr;
                    /*std::cout << "[ EcalProcessFilter ]: " 
                              << " Brem list is empty --> Killing all tracks!"
                              << std::endl;*/
                    return;
                } else { 
                    currentTrack_ = track; 
                    track->SetTrackStatus(fSuspend);
                    TargetBremFilter::removeBremFromList(track);
                    /*std::cout << "[ EcalProcessFilter ]: " 
                              << " Other tracks still need to be processed --> Suspending track!"
                              << std::endl;*/
                    return;
                }
            }
            
            std::cout << "[ EcalProcessFilter ]: "
                      << "Brem photon produced " << secondaries->size() 
                      << " particle via " << processName << " process." 
                      << std::endl;
            TargetBremFilter::removeBremFromList(track);
            BiasingMessenger::setEventWeight(track->GetWeight());
            photonGammaID_ = track->GetTrackID(); 
        }
    }

    void EcalProcessFilter::postTracking(const G4Track* track) { 
       
        if (track->GetParentID() == photonGammaID_) { 
            UserTrackInformation* userInfo 
              = dynamic_cast<UserTrackInformation*>(track->GetUserInformation());
            userInfo->setSaveFlag(true); 
            // get the PDGID of the track.
            G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();
            G4ThreeVector pvec = track->GetMomentum();
            std::cout << "[ EcalProcessFilter ]:\n" 
                      << "\tPDG ID: " << pdgID << "\n"
                      << "\tTrack ID: " << track->GetTrackID() << "\n" 
                      << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                      << "\tParent ID: " << track->GetParentID() << "\n"
                      << "\t p: [ " << pvec[0] << ", " << pvec[1] << ", " << pvec[2] << " ]" << std::endl;
        }
    }
}
