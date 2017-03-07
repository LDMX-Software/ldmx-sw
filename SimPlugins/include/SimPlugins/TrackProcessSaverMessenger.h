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

    class TrackProcessSaverMessenger : public UserActionPluginMessenger {

        public:

            TrackProcessSaverMessenger(TrackProcessSaver* plugin);

            virtual ~TrackProcessSaverMessenger();

            void SetNewValue(G4UIcommand *command, G4String newValue);

        private:

            TrackProcessSaver* plugin_;
            G4UIcommand* addProcessCmd_;

    };
}

#endif /* SIMPLUGINS_TRACKPROCESSSAVERPLUGIN_H_ */
