#ifndef SIMPLUGINS_USERACTIONPLUGINMESSENGER_H_
#define SIMPLUGINS_USERACTIONPLUGINMESSENGER_H_

// LDMX
#include "SimPlugins/UserActionPlugin.h"

// Geant4
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"

namespace sim {

class UserActionPluginMessenger : public G4UImessenger {

    public:

        UserActionPluginMessenger(UserActionPlugin* userPlugin);

        virtual ~UserActionPluginMessenger() {
            delete verboseCmd_;
            delete pluginDir_;
        }

        void SetNewValue(G4UIcommand *command, G4String newValue);

        const std::string& getPath() {
            return pluginDir_->GetCommandPath();
        }

        UserActionPlugin* getPlugin() {
            return userPlugin_;
        }

    private:

        UserActionPlugin* userPlugin_;
        G4UIdirectory* pluginDir_;
        G4UIcommand* verboseCmd_;
};


} // namespace sim

#endif
