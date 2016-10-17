#ifndef SimApplication_TrajectoryContainer_h
#define SimApplication_TrajectoryContainer_h

// Geant4
#include "G4TrajectoryContainer.hh"

// LDMX
#include "SimApplication/Trajectory.h"

// STL
#include <map>

class TrajectoryContainer : public G4TrajectoryContainer {

    public:

        Trajectory* findByTrackID(G4int);
};

#endif
