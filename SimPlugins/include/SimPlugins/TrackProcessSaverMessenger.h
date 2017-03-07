/*
 * TrackProcessSaverMessenger.h
 * @brief Messenger class for TrackProcessSaver plugin
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMPLUGINS_TRACKPROCESSSAVERPLUGIN_H_
#define SIMPLUGINS_TRACKPROCESSSAVERPLUGIN_H_

#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx {

    class TrackProcessSaver;

    /**
     * @class TrackProcessSaverMessenger
     * @brief Messenger class for TrackProcessSaver plugin
     */
    class TrackProcessSaverMessenger : public UserActionPluginMessenger {

        public:

            TrackProcessSaverMessenger(TrackProcessSaver* plugin);

            virtual ~TrackProcessSaverMessenger();

            /**
             * Handle a messenger command.
             * @param command The command being handled.
             * @param newValue The parameter values.
             */
            void SetNewValue(G4UIcommand *command, G4String newValue);

        private:

            /** The associated TrackProcessSaver plugin. */
            TrackProcessSaver* plugin_;

            /** Command for adding a physics process by name for track saving. */
            G4UIcommand* addProcessCmd_;

    };
}

#endif /* SIMPLUGINS_TRACKPROCESSSAVERPLUGIN_H_ */
