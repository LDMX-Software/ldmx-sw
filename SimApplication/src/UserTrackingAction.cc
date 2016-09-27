#include "SimApplication/UserTrackingAction.h"

// LDMX
#include "SimApplication/UserRegionInformation.h"
#include "SimApplication/UserTrackInformation.h"

// STL
#include <iostream>

UserTrackingAction::UserTrackingAction() {
}

UserTrackingAction::~UserTrackingAction() {
}

void UserTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

    // Setup the user track info.
    if (aTrack->GetUserInformation() == NULL) {
        UserTrackInformation* trackInfo = new UserTrackInformation(aTrack);
        const_cast<G4Track*>(aTrack)->SetUserInformation(trackInfo);
    }

    // Turn off save flag for tracks originating in a region where trajectories are not being saved.
    UserRegionInformation* regionInfo =
            (UserRegionInformation*) aTrack->GetLogicalVolumeAtVertex()->GetRegion()->GetUserInformation();
    if (regionInfo != NULL) {
        if (!regionInfo->getStoreSecondaries()) {
            UserTrackInformation* trackInfo = (UserTrackInformation*) const_cast<G4Track*>(aTrack)->GetUserInformation();
            trackInfo->getTrackSummary()->setSaveFlag(false);
        }
    }
}

void UserTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
    ((UserTrackInformation*)aTrack->GetUserInformation())->getTrackSummary()->update(aTrack);
}
