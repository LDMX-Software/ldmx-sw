/**
 * @file UserTrackingAction.h
 * @brief Class which implements the user tracking action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USERTRACKINGACTION_H_
#define SIMAPPLICATION_USERTRACKINGACTION_H_

// LDMX
#include "SimPlugins/PluginManagerAccessor.h"

// Geant4
#include "G4UserTrackingAction.hh"

namespace ldmx {

/**
 * @class UserTrackingAction
 * @brief Implementation of user tracking action
 */
class UserTrackingAction :
        public G4UserTrackingAction,
        public PluginManagerAccessor {

    public:

        /**
         * Class constructor.
         */
        UserTrackingAction();

        /**
         * Class destructor.
         */
        virtual ~UserTrackingAction();

    public:

        /**
         * Implementation of pre-tracking action.
         * @param aTrack The Geant4 track.
         */
        void PreUserTrackingAction(const G4Track* aTrack);

        /**
         * Implementation of post-tracking action.
         * @param aTrack The Geant4 track.
         */
        void PostUserTrackingAction(const G4Track* aTrack);
};

}

#endif
