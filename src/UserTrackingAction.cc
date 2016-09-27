#include "SimApplication/UserTrackingAction.h"

// LDMX
#include "SimApplication/UserTrackInformation.h"

// STL
#include <iostream>

UserTrackingAction::UserTrackingAction() {
}

UserTrackingAction::~UserTrackingAction() {
}

void UserTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {
    if (aTrack->GetUserInformation() == NULL) {
        UserTrackInformation* trackInfo = new UserTrackInformation(aTrack);
        const_cast<G4Track*>(aTrack)->SetUserInformation(trackInfo);
    }
}

void UserTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
    ((UserTrackInformation*)aTrack->GetUserInformation())->getTrackSummary()->update(aTrack);
}
