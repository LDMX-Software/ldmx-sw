/**
 * @file UserEventAction.h
 * @brief Class which implements the Geant4 user event action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USEREVENTACTION_H
#define SIMCORE_USEREVENTACTION_H

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
 * @class UserEventAction
 * @brief Implementation of user event action hook
 */
class UserEventAction : public G4UserEventAction {
 public:
  /**
   * Class constructor.
   */
  UserEventAction() {}

  /**
   * Class destructor.
   */
  virtual ~UserEventAction() {}

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

};  // UserEventAction

}  // namespace simcore

#endif  // SIMCORE_USEREVENTACTION_H
