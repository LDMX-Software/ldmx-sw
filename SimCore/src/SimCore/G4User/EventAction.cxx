
#include "SimCore/G4User/EventAction.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/TrackMap.h"
#include "SimCore/G4User/TrackingAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"

namespace simcore {
namespace g4user {

void EventAction::BeginOfEventAction(const G4Event* event) {
  // Clear the global track map.
  simcore::g4user::TrackingAction::get()->getTrackMap().clear();

  // Call user event actions
  for (auto& event_action : event_actions_) {
    event_action->BeginOfEventAction(event);
  }
}

void EventAction::EndOfEventAction(const G4Event* event) {
  // Call user event actions
  for (auto& event_action : event_actions_) {
    event_action->EndOfEventAction(event);
  }
}

}  // namespace g4user
}  // namespace simcore
