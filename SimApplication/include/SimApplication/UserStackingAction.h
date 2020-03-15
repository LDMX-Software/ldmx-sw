/**
 * @file UserStackingAction.h
 * @brief Class which implements the Geant4 user stacking action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USERSTACKINGACTION_H
#define SIMAPPLICATION_USERSTACKINGACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <vector>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4UserStackingAction.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserAction.h" 

namespace ldmx {

    /**
     * @class UserStackingAction
     * @brief User stacking action implementation
     */
    class UserStackingAction : public G4UserStackingAction {

        public:

            /**
             * Class constructor.
             */
            UserStackingAction() {
            }

            /**
             * Class destructor.
             */
            virtual ~UserStackingAction() {
            }

            /**
             * Classify a new track.
             * @param aTrack The track to classify.
             * @return The track classification.
             */
            //G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track *aTrack);

            /**
             * Invoked when there is a new stacking stage.
             */
            void NewStage();

            /**
             * Invoked for a new event.
             */
            void PrepareNewEvent();

            /**
             * Register a user action of type stacking action with this class. 
             *
             * @param action  User action of type StackingAction
             */
            void registerAction(StackingAction* stackingAction) { stackingActions_.push_back(stackingAction); }

        private: 

            std::vector<StackingAction*> stackingActions_; 
    };

}

#endif
