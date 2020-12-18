
#include "SimCore/UserEventAction.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/RunManager.h"
#include "SimCore/TrackMap.h"
#include "SimCore/UserTrackingAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4RunManager.hh"


namespace ldmx {

    void UserEventAction::BeginOfEventAction(const G4Event* event) {

        // Clear the global track map.
        UserTrackingAction::getUserTrackingAction()->getTrackMap()->clear();

        // Call user event actions
        for ( auto& eventAction : eventActions_ ) eventAction->BeginOfEventAction(event); 
    }

    void UserEventAction::EndOfEventAction(const G4Event* event) {
        // Call user event actions
        for ( auto& eventAction : eventActions_ ) eventAction->EndOfEventAction(event); 
    }

}
