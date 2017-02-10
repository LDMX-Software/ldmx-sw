/**
 * @file ECalPhotonuclearBiasing.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "Biasing/ECalPhotonuclearBiasingPlugin.h"

namespace ldmx { 

    extern "C" ECalPhotonuclearBiasingPlugin* createECalPhotonuclearBiasingPlugin() {
        return new ECalPhotonuclearBiasingPlugin;
    }

    extern "C" void destroyECalPhotonuclearBiasingPlugin(ECalPhotonuclearBiasingPlugin* object) {
        delete object;
    }


    ECalPhotonuclearBiasingPlugin::ECalPhotonuclearBiasingPlugin() { 
    }

    ECalPhotonuclearBiasingPlugin::~ECalPhotonuclearBiasingPlugin() { 
    }


    G4ClassificationOfNewTrack ECalPhotonuclearBiasingPlugin::stackingClassifyNewTrack(
            const G4Track* track, 
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

        if (track->GetTrackID() == 1 && pdgID == 11) {
            /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: Pushing track to waiting stack." << std::endl;*/
            return fWaiting; 
        }

        return classification;
    }

    void ECalPhotonuclearBiasingPlugin::stepping(const G4Step* step) { 

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // Only process the primary particle (parent ID == 0) and its daughters
        // (parent ID == 1)
        if (track->GetParentID() > 1) return;

        /*std::cout << "************" << std::endl;*/
        /*std::cout << "*   Step   *" << std::endl;*/
        /*std::cout << "************" << std::endl;*/ 

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
        if (track->GetParentID() == 0 
                && track->GetCurrentStepNumber() == 1
                && volumeName.compareTo(targetVolumeName_) == 0) {

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

            G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 

            /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                        << "Incident electron produced " << secondaries->size() 
                        << " particle via " << processName << " process." 
                        << std::endl;*/

            // If secondaries were produced via a process other than brem, stop 
            // tracking all tracks.
            if (processName.compareTo("eBrem") != 0 || secondaries->size() != 1) {


                /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                            << "The secondaries are not a result of Brem. --> Killing all tracks!"
                            << std::endl;*/

                track->SetTrackStatus(fKillTrackAndSecondaries);

                G4RunManager::GetRunManager()->AbortEvent();
                return;
            } 

            G4Track* secondaryTrack = secondaries->at(0);
            /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                        << secondaryTrack->GetParticleDefinition()->GetParticleName() 
                        << " with kinetic energy " << secondaryTrack->GetKineticEnergy()
                        << " MeV was produced." << std::endl;*/

            if (secondaryTrack->GetKineticEnergy() < photonEnergyThreshold_) { 

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
        }

        if (track->GetTrackID() == 2 && pdgID == 22) {

            if (secondaries->size() != 0 
                    && (volumeName.contains("W") || volumeName.contains("Si")) 
                    && volumeName.contains("phys")) { 

                G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
                /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                            << "Brem photon produced " << secondaries->size() 
                            << " particles via " << processName << " process within "
                            << secondaries->at(0)->GetVolume()->GetName() << "." 
                            << std::endl;*/

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
            } else if (secondaries->size() != 0) { 
                /*std::cout << "[ ECalPhotonuclearBiasingPlugin ]: "
                            << "Secondaries were produced outside of the ECal --> Killing all tracks!"
                            << std::endl;*/

                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            } else if (volumeName.contains("hcal")) { 
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
        }
    }

}
