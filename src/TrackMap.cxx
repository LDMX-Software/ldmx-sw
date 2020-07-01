#include "SimCore/TrackMap.h"

// Geant4
#include "G4EventManager.hh"
#include "G4Event.hh"

// LDMX
#include "SimCore/Trajectory.h"

namespace ldmx {

    int TrackMap::findIncident(G4int trackID) {
        int currTrackID = trackID;
        bool foundIncident = false;
        while ( not foundIncident ) {
            if ( contains(currTrackID) ) {
                //currTrackID is not a primary and has parent stored
                currTrackID = getParent(currTrackID);
                if ( hasTrajectory(currTrackID) ) {
                    //curr track ID is being stored
                    //TODO if ( originated outside CalorimterRegion ) {
                        //curr track originated outside cal region
                        foundIncident = true;
                    //}
                }
            } else {
                //curr Track ID is a primary and has already been
                //checked above, give up
                foundIncident = true;
            }
        }
        return currTrackID;
    }

    void TrackMap::clear() {
        trajectoryMap_.clear();
        trackIDMap_.clear();
    }
}
