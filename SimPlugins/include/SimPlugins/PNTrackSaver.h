#ifndef SIMPLUGINS_PNTRACKSAVER_H_
#define SIMPLUGINS_PNTRACKSAVER_H_

#include "G4VProcess.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "SimCore/UserTrackInformation.h"

namespace ldmx {

    class PNTrackSaver : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            PNTrackSaver() {;}

            /**
             * Class destructor.
             */
            ~PNTrackSaver() {;}

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "PNTrackSaver";
            }

            bool hasTrackingAction() {
                return true;
            }

            void preTracking(const G4Track* aTrack) {
                const G4VProcess* p = aTrack->GetCreatorProcess();
                if (p && p->GetProcessName() == "photonNuclear") {
                    std::cout << "[ PNTrackSaver ] : flagging PN track with ID " << aTrack->GetTrackID() << std::endl;
                    std::cout << "  processType: " << p->GetProcessType() << std::endl;
                    std::cout << "  processSubType: " << p->GetProcessSubType() << std::endl;
                    dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation())->setSaveFlag(true);
                }
            }
    };
}

#endif

