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

    void EcalProcessFilter::stepping(const G4Step* step) { 

        /*std::cout << "************" << std::endl;*/
        /*std::cout << "*   Step   *" << std::endl;*/
        /*std::cout << "************" << std::endl;*/ 

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();

        // Get the kinetic energy of the particle.
        //double incidentParticleEnergy = step->GetPreStepPoint()->GetTotalEnergy();

        /*std::cout << "[ EcalProcessFilter ]:\n" 
                    << "\tTotal energy of " << particleName  << ": " << incidentParticleEnergy << " MeV \n"
                    << "\tPDG ID: " << pdgID << "\n"
                    << "\tTrack ID: " << track->GetTrackID() << "\n" 
                    << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                    << "\tParent ID: " << track->GetParentID() << "\n"
                    << "\tParticle currently in " << volumeName  << std::endl;*/

        // Get the particles daughters.
        const G4TrackVector* secondaries = step->GetSecondary();

        if (track->GetTrackID() == 2 && pdgID == 22) {

            if (secondaries->size() != 0 
                    && (volumeName.contains("W") || volumeName.contains("Si")) 
                    && volumeName.contains("phys")) { 

                G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
                /*std::cout << "[ EcalProcessFilter ]: "
                            << "Brem photon produced " << secondaries->size() 
                            << " particles via " << processName << " process within "
                            << secondaries->at(0)->GetVolume()->GetName() << "." 
                            << std::endl;*/

                // Only record photonuclear events
                if (!processName.contains(BiasingMessenger::getProcess())) {

                    /*std::cout << "[ EcalProcessFilter ]: "
                                << "Brem photon produced " << secondaries->size() 
                                << " particles via " << processName << " process within "
                                << secondaries->at(0)->GetVolume()->GetName() << "." 
                                << std::endl;*/

                    track->SetTrackStatus(fKillTrackAndSecondaries);
                    G4RunManager::GetRunManager()->AbortEvent();
                    return;
                }

                std::cout << "[ EcalProcessFilter ]: "
                    << "Brem photon produced " << secondaries->size() 
                    << " particles via " << processName << " process within "
                    << secondaries->at(0)->GetVolume()->GetName() << "." 
                    << std::endl;
            } else if (secondaries->size() != 0) { 
                /*std::cout << "[ EcalProcessFilter ]: "
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
