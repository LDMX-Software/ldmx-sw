/**
 * @file UserStackingAction.h
 * @brief Class which implements the Geant4 user stacking action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USERSTACKINGACTION_H
#define SIMAPPLICATION_USERSTACKINGACTION_H

// LDMX
#include "SimPlugins/PluginManagerAccessor.h"

// Geant4
#include "G4UserStackingAction.hh"

namespace sim {

/**
 * @class UserStackingAction
 * @brief User stacking action implementation
 */
class UserStackingAction :
        public G4UserStackingAction,
        public PluginManagerAccessor {

    public:

        /**
         * Class constructor.
         */
        UserStackingAction() {;}

        /**
         * Class destructor.
         */
        virtual ~UserStackingAction() {;}

        /**
         * Classify a new track.
         * @param aTrack The track to classify.
         * @return The track classification.
         */
        G4ClassificationOfNewTrack ClassifyNewTrack (const G4Track *aTrack);

        /**
         * Invoked when there is a new stacking stage.
         */
        void NewStage();

        /**
         * Invoked for a new event.
         */
        void PrepareNewEvent();
};

}

#endif
