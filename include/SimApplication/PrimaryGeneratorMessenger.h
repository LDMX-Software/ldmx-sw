/**
 * @file PrimaryGeneratorMessenger.h
 * @brief Class providing a macro messenger for event generation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_
#define SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"

// LDMX
#include "SimApplication/PrimaryGeneratorAction.h"

namespace ldmx {

/**
 * @class PrimaryGeneratorMessenger
 * @brief Macro messenger for event generation
 */
class PrimaryGeneratorMessenger : public G4UImessenger {

    public:

        /**
         * Class constructor.
         * @param pga The primary generator action.
         */
        PrimaryGeneratorMessenger(PrimaryGeneratorAction* pga);

        /**
         * Class destructor.
         */
        virtual ~PrimaryGeneratorMessenger();

        /**
         * Process macro command.
         * @param command The applicable UI command.
         * @param newValues The argument values.
         */
        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        /**
         * The primary generator action.
         */
        PrimaryGeneratorAction* primaryGeneratorAction_;

        /**
         * The LHE generator macro directory.
         */
        G4UIdirectory* lheDir_;

        /**
         * The command for opening LHE files.
         */
        G4UIcommand* lheOpenCmd_;
};

}

#endif
