/**
 * @file PluginMessenger.h
 * @brief Class defining a messenger for simulation plugins
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_PLUGINMESSENGER_H_
#define SIMPLUGINS_PLUGINMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"

// LDMX
#include "SimPlugins/PluginManager.h"

namespace ldmx {

    /**
     * @class PluginMessenger
     * @brief Messenger class for loading and destroying user sim plugins using macro commands
     */
    class PluginMessenger : public G4UImessenger {

        public:

            /**
             * Class constructor.
             * @param mgr The plugin manager.
             */
            PluginMessenger(PluginManager* mgr);

            /**
             * Class destructor.
             */
            virtual ~PluginMessenger();

            /**
             * Process the macro command.
             * @param[in] command The macro command.
             * @param[in] newValues The argument values.
             */
            void SetNewValue(G4UIcommand* command, G4String newValues);

        private:

            /**
             * The plugin manager.
             */
            PluginManager* pluginManager_;

            /**
             * Directory for plugin commands.
             */
            G4UIdirectory* pluginDir_;

            /**
             * Command for loading a plugin by name.
             */
            G4UIcommand* loadCmd_;

            /**
             * Command for destroying a plugin by name.
             */
            G4UIcommand* destroyCmd_;

            /**
             * Command for listing currently registered plugins.
             */
            G4UIcommand* listCmd_;
    };

}

#endif
