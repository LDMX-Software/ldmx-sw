/**
 * @file TargetPhotonuclearBiasing.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the target.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "SimPlugins/TargetPhotonuclearBiasingPlugin.h"

extern "C" sim::TargetPhotonuclearBiasingPlugin* createTargetPhotonuclearBiasingPlugin() {
    return new sim::TargetPhotonuclearBiasingPlugin;
}

extern "C" void destroyTargetPhotonuclearBiasingPlugin(sim::TargetPhotonuclearBiasingPlugin* object) {
    delete object;
}


sim::TargetPhotonuclearBiasingPlugin::TargetPhotonuclearBiasingPlugin() { 
}

sim::TargetPhotonuclearBiasingPlugin::~TargetPhotonuclearBiasingPlugin() { 
}


G4ClassificationOfNewTrack sim::TargetPhotonuclearBiasingPlugin::stackingClassifyNewTrack(const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {

    /*std::cout << "********************************" << std::endl;*/ 
    /*std::cout << "*   Track pushed to the stack  *" << std::endl;*/
    /*std::cout << "********************************" << std::endl;*/ 

    // get the PDGID of the track.
    G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

    // Get the particle type.
    G4String particleName = track->GetParticleDefinition()->GetParticleName();

    /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: " << "\n" 
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

void sim::TargetPhotonuclearBiasingPlugin::stepping(const G4Step* step) { 

    
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
    
    /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: " << "\n" 
              << "\tTotal energy of " << particleName      << " ( PDG ID: " << pdgID
              << " ) : " << incidentParticleEnergy       << "\n"
              << "\tTrack ID: " << track->GetTrackID()     << "\n" 
              << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
              << "\tParticle currently in " << volumeName  << std::endl;*/

    // Get the particles daughters.
    const G4TrackVector* secondaries = step->GetSecondary();
    
    // process primary track
    if (track->GetTrackID() == 1 && pdgID == 11 && track->GetCurrentStepNumber() == 1) {

        if (volumeName.compareTo(volumeName_) == 0) {

            // If the initial interaction didn't result in any secondaries e.g.
            // a brem photon, don't bother processing the rest of the event.
            if (secondaries->size() == 0) {     
                track->SetTrackStatus(fKillTrackAndSecondaries); 
                
                /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
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
            
            /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                      << secondaryTrack->GetParticleDefinition()->GetParticleName() 
                      << " with kinetic energy " << secondaryTrack->GetKineticEnergy()
                      << " MeV was produced." << std::endl;*/
            
            if (secondaryTrack->GetKineticEnergy() < photonEnergyThreshold_) { 

                
                /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                          << "Brem photon failed the energy threshold cut." 
                          << std::endl;*/ 
                
                track->SetTrackStatus(fKillTrackAndSecondaries);

                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
            
            /*std::cout << "[ TargetPhotonuclerBiasingPlugin ]: "
                      << "Photon passed energy cut --> Suspending primary track!"
                      << std::endl;*/
           track->SetTrackStatus(fSuspend);   
        
        }
    }

    if (track->GetTrackID() == 2 && pdgID == 22  && volumeName.compareTo(volumeName_) == 0) {
        
        // If the brem photon doesn't undergo any reaction in the target, stop
        // processing the rest of the event. 
        if (secondaries->size() == 0) {
            
            
            /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                      << "Brem photon did not interact in the target. --> Killing all tracks!"
                      << std::endl;*/
            
            track->SetTrackStatus(fKillTrackAndSecondaries);
            G4RunManager::GetRunManager()->AbortEvent();
            return;
        }  
    
        G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
        
        
        /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                  << "Brem photon produced " << secondaries->size() 
                  << " particle via " << processName << " process." 
                  << std::endl;*/
        
    
        // Only record photonuclear events
        if (processName.compareTo("photonNuclear") != 0) {
            
            
            /*std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                      << "Process was not photonuclear. --> Killing all tracks!" 
                      << std::endl;*/ 
            
            track->SetTrackStatus(fKillTrackAndSecondaries);

            G4RunManager::GetRunManager()->AbortEvent();
            return;
        }
    
        std::cout << "[ TargetPhotonuclearBiasingPlugin ]: "
                  << "Brem photon produced " << secondaries->size() 
                  << " particle via " << processName << " process." 
                  << std::endl;
    }
}

