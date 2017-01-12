/**
 * @file RootPersistencyMessenger.h
 * @brief Class providing macro commands for ROOT persistency
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_ROOTPERSISTENCYMESSENGER_H_
#define SIMAPPLICATION_ROOTPERSISTENCYMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"
#include "G4UIcmdWithABool.hh"

// LDMX
#include "SimApplication/RootPersistencyManager.h"

namespace sim {

/**
 * @class RootPersistencyMessenger
 * @brief Provides macro commands for RootPersistencyManager
 */
class RootPersistencyMessenger : public G4UImessenger {

    public:

        /**
         * Class constructor.
         * @param rootIO The ROOT persistency manager.
         */
        RootPersistencyMessenger(RootPersistencyManager* rootIO);

        /**
         * Class destructor.
         */
        virtual ~RootPersistencyMessenger();

        /**
         * Process macro command.
         * @param command The applicable UI command.
         * @param newValues The argument values.
         */
        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        /**
         * ROOT persistency macro directory.
         */
        G4UIdirectory* rootDir_;

        /**
         * Command definitions.
         */
        G4UIcommand* rootFileCmd_;
        G4UIcommand* verboseCmd_;
        G4UIcommand* disableCmd_;
        G4UIcommand* enableCmd_;
        G4UIcommand* comprCmd_;
        //G4UIcommand* modeCmd_;
        G4UIcmdWithABool* hitContribsCmd_;
        G4UIcmdWithABool* compressContribsCmd_;
        
        /**
         * Pointer to ROOT persistency manager.
         */
        RootPersistencyManager* rootIO_;
};

}

#endif
