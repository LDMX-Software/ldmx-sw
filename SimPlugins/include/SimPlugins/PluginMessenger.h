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

        PluginManager* pluginManager_;

        G4UIdirectory* pluginDir_;
        G4UIcommand* loadCmd_;
        G4UIcommand* destroyCmd_;
        G4UIcommand* listCmd_;
};

}

#endif
