#include "SimApplication/UserTrackingAction.h"

// LDMX
#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "SimApplication/UserRegionInformation.h"
#include "SimCore/UserTrackInformation.h"
#include "SimPlugins/PluginManager.h"

// Geant4
#include "G4PrimaryParticle.hh"
#include "G4TrackingManager.hh"
#include "G4VUserPrimaryParticleInformation.hh"

// STL
#include <iostream>

namespace ldmx {

    void UserTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

        // Process a single track using the utility method.
        processTrack(aTrack);

        // Activate user plugins.
        pluginManager_->preTracking(aTrack);
    }

    void UserTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {

        // Activate user plugins.
        pluginManager_->postTracking(aTrack);

        // Save extra trajectories that were flagged during event processing.
        if (dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation())->getSaveFlag()) {
            if (!trackMap_.hasTrajectory(aTrack->GetTrackID())) {
                storeTrajectory(aTrack);
            }
        }
    }

    void UserTrackingAction::storeTrajectory(const G4Track* aTrack) {

        // Create a new trajectory for this track.
        fpTrackingManager->SetStoreTrajectory(true);
        Trajectory* traj = new Trajectory(aTrack);
        fpTrackingManager->SetTrajectory(traj);

        // Update the gen status from the primary particle.
        if (aTrack->GetDynamicParticle()->GetPrimaryParticle() != NULL) {
            G4VUserPrimaryParticleInformation* primaryInfo = aTrack->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation();
            if (primaryInfo != NULL) {
                traj->setGenStatus(((UserPrimaryParticleInformation*) primaryInfo)->getHepEvtStatus());
            }
        }

        // Map track ID to trajectory.
        trackMap_.addTrajectory(traj);
    }

    void UserTrackingAction::processTrack(const G4Track* aTrack) {

       // Set user track info on new track.
       const_cast<G4Track*>(aTrack)->SetUserInformation(new UserTrackInformation);

        // Check if trajectory storage should be turned on or off from the region info.
        UserRegionInformation* regionInfo =
                (UserRegionInformation*) aTrack->GetLogicalVolumeAtVertex()->GetRegion()->GetUserInformation();

        if (regionInfo != NULL && !regionInfo->getStoreSecondaries()) {
            // Turn off trajectory storage for this track.
            fpTrackingManager->SetStoreTrajectory(false);
        } else {
            // Store a trajectory for this track.
            storeTrajectory(aTrack);
        }

        // Save the association between track ID and its parent ID for all tracks in the event.
        trackMap_.addSecondary(aTrack->GetTrackID(), aTrack->GetParentID());
    }
}
