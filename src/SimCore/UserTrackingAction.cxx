#include "SimCore/UserTrackingAction.h"

// LDMX
#include "SimCore/TrackMap.h"
#include "SimCore/Trajectory.h"
#include "SimCore/UserPrimaryParticleInformation.h"
#include "SimCore/UserRegionInformation.h"
#include "SimCore/UserTrackInformation.h"

// Geant4
#include "G4PrimaryParticle.hh"
#include "G4TrackingManager.hh"
#include "G4VUserPrimaryParticleInformation.hh"

// STL
#include <iostream>

namespace simcore {

void UserTrackingAction::PreUserTrackingAction(const G4Track* track) {
  int trackID = track->GetTrackID();

  if (trackMap_.contains(trackID)) {
    /**
     * When a track is suspended and then put onto the waiting stack
     * (like what is done using the PartialEnergySorter)
     * The track ends up being passed through the user tracking action
     * more than one time. We need this here to make sure Geant4's
     * trajectory manager has the same setting for whether we should keep
     * the trajectroy as our trajectory manager (TrackMap)
     */
    fpTrackingManager->SetStoreTrajectory(trackMap_.hasTrajectory(trackID));
  } else {
    // New Track

    // Set user track info on new track.
    if (!track->GetUserInformation()) {
      auto trackInfo = new UserTrackInformation;
      trackInfo->setInitialMomentum(track->GetMomentum());
      const_cast<G4Track*>(track)->SetUserInformation(trackInfo);
      trackInfo->setVertexVolume(track->GetVolume()->GetName());
    }

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
    if (curGenStatus == 1) {
      storeTrajectory(track);
    } else if (regionInfo && !regionInfo->getStoreSecondaries()) {
      // Turn off trajectory storage for this track from region flag.
      fpTrackingManager->SetStoreTrajectory(false);
    } else {
      // Store a new trajectory for this track.
      storeTrajectory(track);
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

  // Save extra trajectories on tracks that were flagged for saving during event
  // processing.
  if (dynamic_cast<UserTrackInformation*>(track->GetUserInformation())
          ->getSaveFlag()) {
    if (!trackMap_.hasTrajectory(track->GetTrackID())) {
      storeTrajectory(track);
    }
  }

  // Set end point momentum on the trajectory.
  if (fpTrackingManager->GetStoreTrajectory()) {
    auto traj = dynamic_cast<Trajectory*>(fpTrackingManager->GimmeTrajectory());
    if (traj) {
      if (track->GetTrackStatus() == G4TrackStatus::fStopAndKill) {
        traj->setEndPointMomentum(track);
      }
    }
  }
}

void UserTrackingAction::storeTrajectory(const G4Track* track) {
  // Create a new trajectory for this track.
  fpTrackingManager->SetStoreTrajectory(true);
  Trajectory* traj = new Trajectory(track);
  fpTrackingManager->SetTrajectory(traj);

  // Update the gen status from the primary particle.
  if (track->GetDynamicParticle()->GetPrimaryParticle() != NULL) {
    G4VUserPrimaryParticleInformation* primaryInfo =
        track->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation();
    if (primaryInfo != NULL) {
      traj->setGenStatus(
          ((UserPrimaryParticleInformation*)primaryInfo)->getHepEvtStatus());
    }
  }

  // Map track ID to trajectory.
  trackMap_.addTrajectory(traj);
}
}  // namespace simcore
