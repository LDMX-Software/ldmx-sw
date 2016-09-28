#include "SimApplication/UserTrackingAction.h"

// LDMX
#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"
#include "SimApplication/UserRegionInformation.h"
//#include "SimApplication/UserTrackInformation.h"

// Geant4
#include "G4TrackingManager.hh"

// STL
#include <iostream>

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
        fpTrackingManager->SetTrajectory(new Trajectory(aTrack));
    }

    // Save association between track ID and its parent ID.
    TrackMap::getInstance()->addSecondary(aTrack->GetTrackID(), aTrack->GetParentID());
}

void UserTrackingAction::PostUserTrackingAction(const G4Track*) {
}
