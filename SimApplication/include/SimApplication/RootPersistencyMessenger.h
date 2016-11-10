#ifndef SIMAPPLICATION_ROOTPERSISTENCYMESSENGER_H_
#define SIMAPPLICATION_ROOTPERSISTENCYMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"

// LDMX
#include "SimApplication/RootPersistencyManager.h"

namespace sim {

    class RootPersistencyMessenger : public G4UImessenger {

        public:

            RootPersistencyMessenger(RootPersistencyManager* rootIO);

            virtual ~RootPersistencyMessenger();

            void SetNewValue(G4UIcommand* command, G4String newValues);

        private:
        
            G4UIdirectory* rootDir_;
            G4UIcommand* rootFileCmd_;
            G4UIcommand* verboseCmd_;
            G4UIcommand* disableCmd_;
            G4UIcommand* enableCmd_;
            G4UIcommand* comprCmd_;
            G4UIcommand* modeCmd_;

            RootPersistencyManager* rootIO_;
    };

}

#endif
