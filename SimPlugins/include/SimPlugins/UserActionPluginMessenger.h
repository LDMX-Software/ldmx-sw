/**
 * @file UserActionPluginMessenger.h
 * @brief Class defining a macro messenger for a UserActionPlugin
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_USERACTIONPLUGINMESSENGER_H_
#define SIMPLUGINS_USERACTIONPLUGINMESSENGER_H_

// LDMX
#include "SimPlugins/UserActionPlugin.h"

// Geant4
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"

namespace sim {

/**
 * @class UserActionPluginMessenger
 * @brief Messenger for sending macro commands to a UserActionPlugin
 *
 * @note
 * By default, this class creates a directory for the plugin and provides
 * a command for setting of the verbose level.  Users can override this
 * class to provide additional commands for their specific plugins.
 */
class UserActionPluginMessenger : public G4UImessenger {

    public:

        /**
         * Class constructor.
         * @param userPlugin The associated UserActionPlugin.
         */
        UserActionPluginMessenger(UserActionPlugin* userPlugin);

        /**
         * Class destructor.
         */
        virtual ~UserActionPluginMessenger() {
            delete verboseCmd_;
            delete pluginDir_;
        }

        /**
         * Process the macro command.
         * @param[in] command The macro command.
         * @param[in] newValue The argument values.
         */
        void SetNewValue(G4UIcommand *command, G4String newValue);

        /**
         * Get the command path (plugin's macro directory).
         * @return The command path.
         */
        const std::string& getPath() {
            return pluginDir_->GetCommandPath();
        }

        /**
         * Get the associated UserActionPlugin.
         * @return The associated UserActionPlugin.
         */
        UserActionPlugin* getPlugin() {
            return userPlugin_;
        }

    private:

        /**
         * The associated UserActionPligin.
         */
        UserActionPlugin* userPlugin_;

        /**
         * The plugin's command directory.
         */
        G4UIdirectory* pluginDir_;

        /**
         * The command for setting verbose level.
         */
        G4UIcommand* verboseCmd_;
};


} // namespace sim

#endif
