#include "SimCore/UserTrackingAction.h"

// LDMX
#include "SimCore/TrackMap.h"
#include "SimCore/UserPrimaryParticleInformation.h"
#include "SimCore/UserRegionInformation.h"
#include "SimCore/UserTrackInformation.h"

// Geant4
#include "G4PrimaryParticle.hh"
#include "G4VUserPrimaryParticleInformation.hh"

// STL
#include <iostream>

namespace simcore {

void UserTrackingAction::PreUserTrackingAction(const G4Track* track) {
  int trackID = track->GetTrackID();

  if (not trackMap_.contains(trackID)) {
    // New Track

    // if the track information is not created yet,
    // we make one here in UserTrackInformatin::getInfo
    UserTrackInformation *track_info{UserTrackInformation::getInfo(track)};

    // Check if trajectory storage should be turned on or off from the region
    // info.
    auto regionInfo = (UserRegionInformation*)track->GetLogicalVolumeAtVertex()
                          ->GetRegion()
                          ->GetUserInformation();

    // Check if trajectory storage should be turned on or off from the gen
    // status info
    int curGenStatus = -1;
    if (track->GetDynamicParticle()->GetPrimaryParticle()) {
      auto primaryInfo = dynamic_cast<UserPrimaryParticleInformation*>(
          track->GetDynamicParticle()
              ->GetPrimaryParticle()
              ->GetUserInformation());
      curGenStatus = primaryInfo->getHepEvtStatus();
    }

    // Always save a particle if it has gen status == 1
    // or if it is in a region that should store secondaries
    if (curGenStatus == 1 or !regionInfo or regionInfo->getStoreSecondaries()) {
      track_info->setSaveFlag(true); 
    }

    // Save the association between track ID and its parent ID for all tracks in
    // the event.
    if (track->GetParentID() > 0)
      trackMap_.addSecondary(track->GetTrackID(), track->GetParentID());
  }

  // Activate user tracking actions
  for (auto& trackingAction : trackingActions_)
    trackingAction->PreUserTrackingAction(track);
}

void UserTrackingAction::PostUserTrackingAction(const G4Track* track) {
  // Activate user tracking actions
  for (auto& trackingAction : trackingActions_)
    trackingAction->PostUserTrackingAction(track);

  /**
   * If a track is to-be saved and it is being killed,
   * save the track into the map. This is where a track
   * is chosen to be put into the output particle map.
   * If its save flag is true **for any reason** at this
   * point, then it will be in the output map.
   */
  auto track_info{
      dynamic_cast<UserTrackInformation*>(track->GetUserInformation())};
  if (track_info->getSaveFlag() and
      track->GetTrackStatus() == G4TrackStatus::fStopAndKill) {
    trackMap_.save(track);
  }
}

}  // namespace simcore
