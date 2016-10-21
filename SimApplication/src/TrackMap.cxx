#include "SimApplication/TrackMap.h"

// Geant4
#include "G4EventManager.hh"
#include "G4Event.hh"

// LDMX
#include "SimApplication/TrajectoryContainer.h"

namespace sim {

TrackMap* TrackMap::getInstance() {
    static TrackMap INSTANCE;
    return &INSTANCE;
}

void TrackMap::addSecondary(G4int trackID, G4int parentID) {
    trackIDMap[trackID] = parentID;
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
            if (trackIDMap.find(currTrackID) != trackIDMap.end()) {
                currTrackID = trackIDMap[currTrackID];
            } else {
                break;
            }
        }
    }
    return traj;
}

void TrackMap::clear() {
    trackIDMap.clear();
}

}

