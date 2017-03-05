#include "SimApplication/TrackMap.h"

// Geant4
#include "G4EventManager.hh"
#include "G4Event.hh"

// LDMX
#include "SimApplication/Trajectory.h"

namespace ldmx {

void TrackMap::addSecondary(G4int trackID, G4int parentID) {
    trackIDMap_[trackID] = parentID;
}

G4VTrajectory* TrackMap::findTrajectory(Trajectory::TrajectoryMap* trajectoryMap, G4int trackID) {
    G4int currTrackID = trackID;
    G4VTrajectory* traj = nullptr;
    for (;;) {
        traj = (*trajectoryMap)[currTrackID];
        if (traj != nullptr) {
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

