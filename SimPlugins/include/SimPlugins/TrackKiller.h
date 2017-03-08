/**
 * @file TrackKiller.h
 * @class TrackKiller
 * @brief Class defining a UserActionPlugin that allows a user to kill all 
 *        tracks in an event.  This is used for testing purposes only.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_TRACKKILLER_H_
#define SIMPLUGINS_TRACKKILLER_H_

// Geant4
#include "G4RunManager.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"

namespace ldmx {

    class TrackKiller : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            TrackKiller();

            /**
             * Class destructor.
             */
            ~TrackKiller();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "TrackKiller";
            }

            /**
             * Get whether this plugin implements the stepping action.
             * @return True to indicate this plugin implements the stepping action.
             */
            bool hasSteppingAction() {
                return true;
            }

            /**
             * Implement the stepping action which kills all tracks.
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step);

    }; // TrackKiller
}

#endif // SIMPLUGINS_TRACKKILLER_H__
