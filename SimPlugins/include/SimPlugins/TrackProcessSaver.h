/**
 * @file TrackProcessSaver.h
 * @brief Class that defines a UserActionPlugin for saving all tracks produced by specified physics processes
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */
#ifndef SIMPLUGINS_TRACKPROCESSSAVER_H_
#define SIMPLUGINS_TRACKPROCESSSAVER_H_

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "SimCore/UserTrackInformation.h"

#include <string>
#include <unordered_set>

namespace ldmx {

    /** Forward declare the messenger class to avoid circular dependency. */
    class TrackProcessSaverMessenger;

    /**
     * @class TrackProcessSaver
     * @brief UserActionPlugin for saving tracks from specified physics processes
     */
    class TrackProcessSaver : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            TrackProcessSaver();

            /**
             * Class destructor.
             */
            ~TrackProcessSaver();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "TrackProcessSaver";
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
            void preTracking(const G4Track* aTrack);

            /**
             * Add the name of a process to save e.g. "photonNuclear".
             * @param processName The name of the physics process.
             */
            void addProcess(std::string processName) {
                processNames_.insert(processName);
            }

        private:

            /** Plugin's Geant4 messenger object. */
            TrackProcessSaverMessenger* messenger_;

            /** Names of physics processes to save. */
            std::unordered_set<std::string> processNames_;
    };
}

#endif

