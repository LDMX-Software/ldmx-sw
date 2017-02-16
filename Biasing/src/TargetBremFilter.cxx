/**
 * @file TargetBremFilter.cxx
 * @class TargetBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a brem within the target.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetBremFilter.h"

namespace ldmx { 

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

        /*std::cout << "********************************" << std::endl;*/ 
        /*std::cout << "*   Track pushed to the stack  *" << std::endl;*/
        /*std::cout << "********************************" << std::endl;*/ 

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        /*std::cout << "[ TargetBremFilter ]: " << "\n" 
                    << "\tParticle " << particleName      << " ( PDG ID: " << pdgID << " ) : " << "\n"
                    << "\tTrack ID: " << track->GetTrackID()     << "\n" 
                    << std::endl;*/


        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;

        if (track->GetTrackID() == 1 && pdgID == 11) {
            return fWaiting; 
        }

        return classification;
    }

    void TargetBremFilter::stepping(const G4Step* step) { 

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

        /*std::cout << "[ TargetBremFilter ]: " << "\n" 
                    << "\tTotal energy of " << particleName      << " ( PDG ID: " << pdgID
                    << " ) : " << incidentParticleEnergy       << "\n"
                    << "\tTrack ID: " << track->GetTrackID()     << "\n" 
                    << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                    << "\tParticle currently in " << volumeName  << std::endl;*/

        // Get the particles daughters.
        const G4TrackVector* secondaries = step->GetSecondary();

        // process primary track
        // TODO: This block of code is shared by the Target and Ecal biasing plugins
        //       so it should eventually be pulled out to it's own plugin.
        if (track->GetTrackID() == 1 && pdgID == 11 && track->GetCurrentStepNumber() == 1) {

            if (volumeName.compareTo(volumeName_) == 0) {

                // If the initial interaction didn't result in any secondaries e.g.
                // a brem photon, don't bother processing the rest of the event.
                if (secondaries->size() == 0) {     
                    track->SetTrackStatus(fKillTrackAndSecondaries); 

                    /*std::cout << "[ TargetBremFilter ]: "
                                << "Primary did not produce secondaries --> Killing primary track!" 
                                << std::endl;*/

                    G4RunManager::GetRunManager()->AbortEvent();
                    return;
                }

                G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 

                /*std::cout << "[ TargetBremFilter ]: "
                            << "Incident electron produced " << secondaries->size() 
                            << " particle via " << processName << " process." 
                            << std::endl;*/


                // If secondaries were produced via a process other than brem, stop 
                // tracking all tracks.
                if (processName.compareTo("eBrem") != 0 || secondaries->size() != 1) {


                    /*std::cout << "[ TargetBremFilter ]: "
                                << "The secondaries are not a result of Brem. --> Killing all tracks!"
                                << std::endl;*/

                    track->SetTrackStatus(fKillTrackAndSecondaries);

                    G4RunManager::GetRunManager()->AbortEvent();
                    return;
                } 

                G4Track* secondaryTrack = secondaries->at(0);

                /*std::cout << "[ TargetBremFilter ]: "
                            << secondaryTrack->GetParticleDefinition()->GetParticleName() 
                            << " with kinetic energy " << secondaryTrack->GetKineticEnergy()
                            << " MeV was produced." << std::endl;*/

                if (secondaryTrack->GetKineticEnergy() < photonEnergyThreshold_) { 


                    /*std::cout << "[ TargetBremFilter ]: "
                                << "Brem photon failed the energy threshold cut." 
                                << std::endl;*/ 

                    track->SetTrackStatus(fKillTrackAndSecondaries);

                    G4RunManager::GetRunManager()->AbortEvent();
                    return;
                }

                /*std::cout << "[ TargetBremFilter ]: "
                            << "Photon passed energy cut --> Suspending primary track!"
                            << std::endl;*/
                track->SetTrackStatus(fSuspend);   

            }
        }
    }
}

