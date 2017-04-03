/**
 * @file TrackSaver.h
 * @brief Class that defines a sim plugin for saving all tracks in the event
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */
#ifndef SIMPLUGINS_TRACKSAVER_H_
#define SIMPLUGINS_TRACKSAVER_H_

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "SimCore/UserTrackInformation.h"

namespace ldmx {

    /**
     * @class TrackSaver
     * @brief UserActionPlugin for saving all tracks in the event
     */
    class TrackSaver : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            TrackSaver() {;}

            /**
             * Class destructor.
             */
            ~TrackSaver() {;}

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "TrackSaver";
            }

            /**
             * Return whether this plugin defines a tracking action (true for this plugin).
             * @return Return whether this plugin defines a tracking action.
             */
            bool hasTrackingAction() {
                return true;
            }

            /**
             * Define the pre-tracking hook, which flags all tracks for saving.
             * @param aTrack The Geant4 track, which will be flagged for saving.
             */
            void preTracking(const G4Track* aTrack) {
                dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation())->setSaveFlag(true);
            }
    };
}

#endif

