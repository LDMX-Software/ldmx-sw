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
            G4UIdirectory* persistencyDir_;
            G4UIdirectory* rootDir_;
            G4UIcommand* rootFileCmd_;

            RootPersistencyManager* rootIO_;
    };

}

#endif
