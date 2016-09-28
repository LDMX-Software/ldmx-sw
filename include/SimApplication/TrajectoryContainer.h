#ifndef SIMAPPLICATION_TRAJECTORYCONTAINER_H_
#define SIMAPPLICATION_TRAJECTORYCONTAINER_H_ 1

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
