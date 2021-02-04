/**
 * @file UserRunAction.h
 * @brief Class which implements user run action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USERRUNACTION_H
#define SIMCORE_USERRUNACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <vector>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4UserRunAction.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

// Forward declarations
class G4Run;

namespace simcore {

/**
 * @class UserRunAction
 * @brief Implementation of user run action hook
 */
class UserRunAction : public G4UserRunAction {
 public:
  /**
   * Class constructor.
   */
  UserRunAction();

  /**
   * Class destructor.
   */
  virtual ~UserRunAction();

  /**
   * Implementation of begin run hook.
   * @param run The current Geant4 run info.
   */
  void BeginOfRunAction(const G4Run* run);

  /**
   * Implementation of end run hook.
   * @param run The current Geant4 run info.
   */
  void EndOfRunAction(const G4Run* run);

  /**
   * Register a user action of type RunAction with this class.
   *
   * @param action  User action of type RunAction
   */
  void registerAction(UserAction* runAction) {
    runActions_.push_back(runAction);
  }

 private:
  std::vector<UserAction*> runActions_;

};  // UserRunAction

}  // namespace simcore

#endif  // SIMCORE_USERRUNACTION_H
