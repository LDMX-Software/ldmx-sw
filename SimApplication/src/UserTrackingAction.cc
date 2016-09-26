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

    std::cout << "PreUserTrackingAction - track ID " << aTrack->GetTrackID() << std::endl;

    if (aTrack->GetUserInformation() == NULL) {
        std::cout << "Creating new track info for track ID " << aTrack->GetTrackID() << std::endl;
        UserTrackInformation* trackInfo = new UserTrackInformation(aTrack);
        std::cout << "Added track info pointer " << (void*) trackInfo << std::endl;
        const_cast<G4Track*>(aTrack)->SetUserInformation(trackInfo);
    }
}

void UserTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
    std::cout << "PostUserTrackingAction - track ID " << aTrack->GetTrackID() << std::endl;
    ((UserTrackInformation*)aTrack->GetUserInformation())->getTrackSummary()->update(aTrack);
}
