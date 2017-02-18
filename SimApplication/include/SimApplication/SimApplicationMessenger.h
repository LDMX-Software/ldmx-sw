/**
 * @file SimApplicationMessenger.h
 * @brief Class defining a macro messenger for the simulation application
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_SIMAPPLICATIONMESSENGER_H_
#define SIMAPPLICATION_SIMAPPLICATIONMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"

namespace ldmx {

/**
 * @class SimApplicationMessenger
 * @brief Macro commands for the simulation application
 *
 * @brief
 * Currently this just defines the base <i>/ldmx</i> macro directory.
 */
class SimApplicationMessenger : public G4UImessenger {

    public:

        /**
         * Class constructor.
         */
        SimApplicationMessenger();

        /**
         * Class destructor.
         */
        virtual ~SimApplicationMessenger();

    public:

        /**
         * Process the macro command.
         * @param[in] command The macro command.
         * @param[in] newValues The argument values.
         */
        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        /**
         * Top-level LDMX directory.
         */
        G4UIdirectory* ldmxDir_;
};

}

#endif
