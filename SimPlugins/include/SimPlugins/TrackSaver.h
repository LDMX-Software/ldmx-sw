#ifndef SIMPLUGINS_TRACKSAVER_H_
#define SIMPLUGINS_TRACKSAVER_H_

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "SimCore/UserTrackInformation.h"

namespace ldmx {

    class TrackSaver : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            TrackSaver();

            /**
             * Class destructor.
             */
            ~TrackSaver();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "TrackSaver";
            }

            bool hasTrackingAction() {
                return true;
            }

            void preTracking(const G4Track* aTrack) {
                UserTrackInformation* userInfo = dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation());
                if (!userInfo->getSaveFlag()) {
                    userInfo->setSaveFlag(true);
                }        
            }
    };
}

#endif

