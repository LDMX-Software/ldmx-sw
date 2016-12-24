#ifndef SIMAPPLICATION_TRAJECTORYCONTAINER_H_
#define SIMAPPLICATION_TRAJECTORYCONTAINER_H_

// Geant4
#include "G4TrajectoryContainer.hh"

// LDMX
#include "SimApplication/Trajectory.h"

// STL
#include <map>

namespace sim {

class TrajectoryContainer : public G4TrajectoryContainer {

    public:

        TrajectoryContainer() {;}
        virtual ~TrajectoryContainer() {;}

        Trajectory* findByTrackID(G4int);
};

}

#endif
