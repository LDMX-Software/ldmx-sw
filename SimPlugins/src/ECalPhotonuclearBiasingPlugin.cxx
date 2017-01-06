/**
 * @file ECalPhotonuclearBiasing.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "SimPlugins/ECalPhotonuclearBiasingPlugin.h"

extern "C" sim::ECalPhotonuclearBiasingPlugin* createECalPhotonuclearBiasingPlugin() {
    return new sim::ECalPhotonuclearBiasingPlugin;
}

extern "C" void destroyECalPhotonuclearBiasingPlugin(sim::ECalPhotonuclearBiasingPlugin* object) {
    delete object;
}


sim::ECalPhotonuclearBiasingPlugin::ECalPhotonuclearBiasingPlugin() { 
}

sim::ECalPhotonuclearBiasingPlugin::~ECalPhotonuclearBiasingPlugin() { 
}


G4ClassificationOfNewTrack sim::ECalPhotonuclearBiasingPlugin::stackingClassifyNewTrack(const G4Track* track, 
        const G4ClassificationOfNewTrack& currentTrackClass) {

    /*std::cout << "********************************" << std::endl;*/ 
    /*std::cout << "*   Track pushed to the stack  *" << std::endl;*/
    /*std::cout << "********************************" << std::endl;*/ 

    // get the PDGID of the track.
    G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

    // Get the particle type.
    G4String particleName = track->GetParticleDefinition()->GetParticleName();

    /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: " << "\n" 
        << "\tParticle " << particleName      << " ( PDG ID: " << pdgID << " ) : " << "\n"
        << "\tTrack ID: " << track->GetTrackID()     << "\n" 
        << std::endl;*/


    // Use current classification by default so values from other plugins are not overridden.
    G4ClassificationOfNewTrack classification = currentTrackClass;

    if (track != targetGamma) {
        /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: Pushing track to waiting stack." << std::endl;*/
        return fWaiting; 
    }

    return classification;
}

void sim::ECalPhotonuclearBiasingPlugin::stepping(const G4Step* step) { 

    // Get the track associated with this step.
    G4Track* track = step->GetTrack();

    // Only process the primary particle (parent ID == 0) and its daughters
    // (parent ID == 1)
    if (track->GetParentID() > 1) return;

    /*std::cout << "************" << std::endl;*/
    /*std::cout << "*   Step   *" << std::endl;*/
    /*std::cout << "************" << std::endl;*/ 


    if (track->GetParentID() == 0 && track->GetCurrentStepNumber() == 1) { 
        targetGamma = nullptr;
    }

    // get the PDGID of the track.
    G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

    // Get the particle type.
    G4String particleName = track->GetParticleDefinition()->GetParticleName();

    // Get the volume the particle is in.
    G4VPhysicalVolume* volume = track->GetVolume();
    G4String volumeName = volume->GetName();

    // Get the kinetic energy of the particle.
    double incidentParticleEnergy = step->GetPreStepPoint()->GetTotalEnergy();

    /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]:\n" 
        << "\tTotal energy of " << particleName  << ": " << incidentParticleEnergy << " MeV \n"
        << "\tPDG ID: " << pdgID << "\n"
        << "\tTrack ID: " << track->GetTrackID() << "\n" 
        << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
        << "\tParent ID: " << track->GetParentID() << "\n"
        << "\tParticle currently in " << volumeName  << std::endl;*/

    // Get the particles daughters.
    const G4TrackVector* secondaries = step->GetSecondary();

    // process primary track
    if ((track->GetParentID() == 0)
            && (volumeName.compareTo(targetVolumeName_) == 0)) {

        if (targetGamma == nullptr) {
            // If the initial interaction didn't result in any secondaries e.g.
            // a brem photon, don't bother processing the rest of the event.
            if (secondaries->size() == 0) {     
                track->SetTrackStatus(fKillTrackAndSecondaries); 

                /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "  
                    << "Primary did not produce secondaries --> Killing primary track!" 
                    << std::endl;*/

                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }

            for (G4Track* secondaryTrack : *secondaries) { 
                /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: Secondary: "
                    << secondaryTrack->GetParticleDefinition()->GetParticleName() 
                    << " Volume: " << secondaryTrack->GetVolume()->GetName()
                    << " Process: " << secondaryTrack->GetCreatorProcess()->GetProcessName()
                    << " Track ID: " << secondaryTrack->GetTrackID()
                    << std::endl;*/        
                if (secondaryTrack->GetVolume()->GetName().compareTo(targetVolumeName_) == 0) { 
                    G4String processName = secondaryTrack->GetCreatorProcess()->GetProcessName();
                    if (secondaryTrack->GetParticleDefinition()->GetPDGEncoding() == 22
                            && processName.compareTo("eBrem") == 0) { 

                        /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: Found PN gamma." << std::endl;*/
                        targetGamma = secondaryTrack; 
                        break;
                    }
                }
            }

            if (targetGamma == nullptr) { 
                /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                    << "A PN gamma was not created in the target --> Killing all tracks!"
                    << std::endl;*/

                track->SetTrackStatus(fKillTrackAndSecondaries);

                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }

            /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                << targetGamma->GetParticleDefinition()->GetParticleName() 
                << " with kinetic energy " << targetGamma->GetKineticEnergy()
                << " MeV was produced." << std::endl;*/

            if (targetGamma->GetKineticEnergy() < photonEnergyThreshold_) { 

                /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                    << "Brem photon failed the energy threshold cut." 
                    << std::endl;*/

                track->SetTrackStatus(fKillTrackAndSecondaries);

                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }

            /*std::cout << "[ ECalPhotonuclerBiasingPlugin ]: "
                << "Photon passed energy cut --> Suspending primary track!"
                << std::endl;*/
            track->SetTrackStatus(fSuspend);   
        } else if (targetGamma != nullptr && secondaries->size() != 0) {
            /*std::cout << "[ ECalPhotonuclerBiasingPlugin ]: "
                      << "Multiple particles were created in the target --> Killing all tracks!"
                      << std::endl;*/
                
            track->SetTrackStatus(fKillTrackAndSecondaries);

            G4RunManager::GetRunManager()->AbortEvent();
            return;
        } 
    } 

    if (track == targetGamma) {
        if (secondaries->size() != 0) { 
            G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
            /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                << "Brem photon produced " << secondaries->size() 
                << " particles via " << processName << " process within "
                << secondaries->at(0)->GetVolume()->GetName() << "." 
                << std::endl;*/
            if ((volumeName.contains("W") || volumeName.contains("Si")) && volumeName.contains("phys")) { 

                // Only record photonuclear events
                if (!processName.contains("photonNuclear")) {

                    /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                        << "Brem photon produced " << secondaries->size() 
                        << " particles via " << processName << " process within "
                        << secondaries->at(0)->GetVolume()->GetName() << "." 
                        << std::endl;*/

                    track->SetTrackStatus(fKillTrackAndSecondaries);

                    G4RunManager::GetRunManager()->AbortEvent();
                    return;
                }

                std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                          << "Brem photon produced " << secondaries->size() 
                          << " particles via " << processName << " process within "
                          << secondaries->at(0)->GetVolume()->GetName() << "." 
                          << std::endl;
            } else { 
                /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                    << "Secondaries were produced outside of the ECal --> Killing all tracks!"
                    << std::endl;*/

                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
        } 
    }
}

