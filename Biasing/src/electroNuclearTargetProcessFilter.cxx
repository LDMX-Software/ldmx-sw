/**
 * @file electroNuclearTargetProcessFilter.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve an electronuclear reaction in the target.
 * @author Omar Moreno--modified by Alex Rickman
 *         SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetProcessFilter.h"

namespace ldmx { 

    extern "C" electroNuclearTargetProcessFilter* createElectroNuclearTargetProcessFilter() {
        return new electroNuclearTargetProcessFilter;
    }

    extern "C" void destroyElectroNuclearTargetProcessFilter(electroNuclearTargetProcessFilter* object) {
        delete object;
    }

    electroNuclearTargetProcessFilter::electroNuclearTargetProcessFilter() {
    }

    electroNuclearTargetProcessFilter::~electroNuclearTargetProcessFilter() {
    }

    G4ClassificationOfNewTrack electroNuclearTargetProcessFilter::stackingClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;
        
        return classification;
    }

    void electroNuclearTargetProcessFilter::stepping(const G4Step* step) { 

        if (reactionOccurred_){
            return;
        }

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // Only process tracks from the primary particle
        if (track->GetParentID() != 0) return; 

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Make sure that the particle being processed is an electron.
        if (pdgID != 11) return; // Throw an exception

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();

        // If the particle isn't in the target, don't continue with the processing.
        if (volumeName.compareTo(volumeName_) != 0) return;

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        // Get the particle's daughters.
        const G4TrackVector* secondaries = step->GetSecondary();

        // If the electron doesn't undergo reaction, stop processing the event. 
        if (secondaries->size() == 0) {
            track->SetTrackStatus(fKillTrackAndSecondaries);
            G4RunManager::GetRunManager()->AbortEvent();
            return;  
        } 
        else { 
            G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
            // Only record the process that is being biased
            if (!processName.contains(BiasingMessenger::getProcess())) {
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;  
            }
            BiasingMessenger::setEventWeight(track->GetWeight());
            reactionOccurred_ = true;
        }
    }    

    void electroNuclearTargetProcessFilter::endEvent(const G4Event*) {
        reactionOccurred_ = false; 
    }
}
