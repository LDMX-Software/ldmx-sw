/**
 * @file UserEventAction.h
 * @brief Class which implements the Geant4 user event action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USEREVENTACTION_H
#define SIMAPPLICATION_USEREVENTACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <vector>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4UserEventAction.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserAction.h" 

// Forward declarations
class G4Event;

namespace ldmx {

    /**
     * @class UserEventAction
     * @brief Implementation of user event action hook
     */
    class UserEventAction : public G4UserEventAction {

        public:

            /**
             * Class constructor.
             */
            UserEventAction() {
            }

            /**
             * Class destructor.
             */
            virtual ~UserEventAction() {
            }

            /**
             * Implementation of begin of event hook.
             * @param anEvent The Geant4 event.
             */
            void BeginOfEventAction(const G4Event* anEvent);

            /**
             * Implementation of end of event hook.
             * @param anEvent The Geant4 event.
             */
            void EndOfEventAction(const G4Event* anEvent);

            /**
             * Register a user action of type EventAction with this class. 
             *
             * @param action  User action of type EventAction
             */
            void registerAction(EventAction* eventAction) { eventActions_.push_back(eventAction); }
        
        private:

            std::vector<EventAction*> eventActions_; 
    
    };  // UserEventAction

}  // ldmx

#endif //
