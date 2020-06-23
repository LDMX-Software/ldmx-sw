#include "SimApplication/TrackMap.h"

// Geant4
#include "G4EventManager.hh"
#include "G4Event.hh"

// LDMX
#include "SimApplication/Trajectory.h"

namespace ldmx {

    G4VTrajectory* TrackMap::findTrajectory(G4int trackID) {
        G4int currTrackID = trackID;
        G4VTrajectory* traj = nullptr;
        for (;;) {
            traj = trajectoryMap_[currTrackID];
            if (traj) {
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

        for (const auto& [trackID, trajectory] : trajectoryMap_) { 
            delete trajectory;
        }
        trajectoryMap_.clear();

        trackIDMap_.clear();
    }
}
