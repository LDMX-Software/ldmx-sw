#ifndef SIMPLUGINS_PLUGINMESSENGER_H_
#define SIMPLUGINS_PLUGINMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"

// LDMX
#include "SimPlugins/PluginManager.h"

namespace sim {

class PluginMessenger : public G4UImessenger {

    public:

        PluginMessenger(PluginManager*);

        virtual ~PluginMessenger();

        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        PluginManager* pluginManager;

        G4UIdirectory* pluginDir;
        G4UIcommand* loadCmd;
        G4UIcommand* destroyCmd;
        G4UIcommand* listCmd;
};

}

#endif
