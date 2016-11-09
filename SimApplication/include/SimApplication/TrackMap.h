#ifndef SIMAPPLICATION_TRACKERMAP_H_
#define SIMAPPLICATION_TRACKERMAP_H_

// Geant4
#include "G4Event.hh"
#include "G4VTrajectory.hh"

namespace sim {

class TrackMap {

    public:

        // Map of track ID to parent ID.
        typedef std::map<G4int, G4int> TrackIDMap;

        static TrackMap* getInstance();

        void addSecondary(G4int trackID, G4int parentID);

        G4VTrajectory* findTrajectory(const G4Event* anEvent, G4int trackID);

        void clear();

    private:

        TrackIDMap trackIDMap_;
};

}

#endif
