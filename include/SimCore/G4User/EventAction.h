/**
 * @file EventAction.h
 * @brief Class which implements the Geant4 user event action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_G4USER_EVENTACTION_H
#define SIMCORE_G4USER_EVENTACTION_H

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
#include "SimCore/UserAction.h"

// Forward declarations
class G4Event;

namespace simcore {

/**
 * @namespace g4user
 * This namespace is meant to contain
 * all the standard user actions that allow
 * a Geant4 user to interface with its internal simulation
 * engine. We have modified how we interact with these
 * places for our own software using UserAction.
 */
namespace g4user {

/**
 * @class EventAction
 * @brief Implementation of user event action hook
 */
class EventAction : public G4UserEventAction {
 public:
  /**
   * Class constructor.
   */
  EventAction() = default;

  /**
   * Class destructor.
   */
  virtual ~EventAction() = default;

  /**
   * Implementation of begin of event hook.
   * @param event The Geant4 event.
   */
  void BeginOfEventAction(const G4Event* event);

  /**
   * Implementation of end of event hook.
   * @param event The Geant4 event.
   */
  void EndOfEventAction(const G4Event* event);

  /**
   * Register a user action of type EventAction with this class.
   *
   * @param action  User action of type EventAction
   */
  void registerAction(UserAction* eventAction) {
    eventActions_.push_back(eventAction);
  }

 private:
  std::vector<UserAction*> eventActions_;

};  // EventAction

}  // namespace g4user
}  // namespace simcore

#endif  // SIMCORE_G4USER_EVENTACTION_H
