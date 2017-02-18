#include "SimApplication/TrackMap.h"

// Geant4
#include "G4EventManager.hh"
#include "G4Event.hh"

// LDMX
#include "SimApplication/TrajectoryContainer.h"

namespace ldmx {

void TrackMap::addSecondary(G4int trackID, G4int parentID) {
    trackIDMap_[trackID] = parentID;
}

G4VTrajectory* TrackMap::findTrajectory(const G4Event* anEvent, G4int trackID) {
    TrajectoryContainer* trajectories = (TrajectoryContainer*)anEvent->GetTrajectoryContainer();
    G4int currTrackID = trackID;
    G4VTrajectory* traj = NULL;
    for (;;) {
        traj = trajectories->findByTrackID(currTrackID);
        if (traj != NULL) {
            break;
        } else {
            if (trackIDMap_.find(currTrackID) != trackIDMap_.end()) {
                currTrackID = trackIDMap_[currTrackID];
            } else {
                break;
            }
        }
    }
    return traj;
}

void TrackMap::clear() {
    trackIDMap_.clear();
}

}

