#include "SimApplication/UserTrackingAction.h"

// LDMX
#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "SimApplication/UserRegionInformation.h"
#include "SimPlugins/PluginManager.h"

// Geant4
#include "G4PrimaryParticle.hh"
#include "G4TrackingManager.hh"
#include "G4VUserPrimaryParticleInformation.hh"

// STL
#include <iostream>

namespace ldmx {

UserTrackingAction::UserTrackingAction() {
}

UserTrackingAction::~UserTrackingAction() {
}

void UserTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

    // Check if trajectory storage should be turned on or off.
    UserRegionInformation* regionInfo =
            (UserRegionInformation*) aTrack->GetLogicalVolumeAtVertex()->GetRegion()->GetUserInformation();
    if (regionInfo != NULL && !regionInfo->getStoreSecondaries()) {
        fpTrackingManager->SetStoreTrajectory(false);
    } else {
        fpTrackingManager->SetStoreTrajectory(true);
        Trajectory* traj = new Trajectory(aTrack);
        fpTrackingManager->SetTrajectory(traj);

        if (aTrack->GetDynamicParticle()->GetPrimaryParticle() != NULL) {
            G4VUserPrimaryParticleInformation* primaryInfo = aTrack->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation();
            if (primaryInfo != NULL) {
                traj->setGenStatus(((UserPrimaryParticleInformation*) primaryInfo)->getHepEvtStatus());
            }
        }
    }

    // Save association between track ID and its parent ID.
    TrackMap::getInstance()->addSecondary(aTrack->GetTrackID(), aTrack->GetParentID());

    pluginManager_->preTracking(aTrack);
}

void UserTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
    pluginManager_->postTracking(aTrack);
}

}
