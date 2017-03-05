/**
 * @file TrackMap.h
 * @brief Class which defines a map of track ID to parent ID
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_TRACKERMAP_H_
#define SIMAPPLICATION_TRACKERMAP_H_

// Geant4
#include "G4Event.hh"
#include "G4VTrajectory.hh"

#include "SimApplication/Trajectory.h"

namespace ldmx {

/**
 * @class TrackMap
 * @brief Defines a map of track ID to parent ID
 *
 * @note
 * This class provides a record of track ancestry which is used
 * to connect track IDs to their parents.
 */
class TrackMap {

    public:

        /**
         * Map of track ID to parent ID.
         */
        typedef std::map<G4int, G4int> TrackIDMap;

        /**
         * Add a record in the map connecting a track ID to its parent ID.
         * @param trackID The track ID.
         * @param parentID The parent track ID.
         */
        void addSecondary(G4int trackID, G4int parentID);

        /**
         * Find a trajectory by its track ID.
         * If this track ID does not have a trajectory, then the 
         * first trajectory found in its parentage is returned.
         * @param anEvent The Geant4 event.
         * @param trackkID The track ID of the trajectory to find.
         */
        G4VTrajectory* findTrajectory(Trajectory::TrajectoryMap* trajectoryMap, G4int trackID);

        /**
         * Clear the map.
         */
        void clear();

    private:

        /**
         * The map of track IDs to parent IDs.
         */
        TrackIDMap trackIDMap_;
};

}

#endif
