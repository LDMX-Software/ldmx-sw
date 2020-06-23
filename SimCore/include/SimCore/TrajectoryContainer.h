/**
 * @file TrajectoryContainer.h
 * @brief Class which implements a Trajectory container
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_TRAJECTORYCONTAINER_H_
#define SIMAPPLICATION_TRAJECTORYCONTAINER_H_

// Geant4
#include "G4TrajectoryContainer.hh"

// LDMX
#include "SimCore/Trajectory.h"

// STL
#include <map>

namespace ldmx {

    /**
     * @class TrajectoryContainer
     * @brief Trajectory container extension that allows searching by track ID
     *
     * @note Not currently used!!!
     */
    class TrajectoryContainer : public G4TrajectoryContainer {

        public:

            /**
             * Class constructor.
             */
            TrajectoryContainer() {;}

            /**
             * Class destructor.
             */
            virtual ~TrajectoryContainer() {;}

            /**
             * Find a trajectory by its track ID.
             * @return The trajectory or <i>nullptr</i> if it does not exist.
             * @todo Speed this up by using a map instead of linear search.
             *
             * @note Replaced by static method in Trajectory class for now.
             */
            Trajectory* findByTrackID(G4int);
    };

}

#endif
