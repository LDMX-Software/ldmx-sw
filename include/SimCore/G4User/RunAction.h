/**
 * @file UserRunAction.h
 * @brief Class which implements user run action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_G4USER_RUNACTION_H
#define SIMCORE_G4USER_RUNACTION_H

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
namespace g4user {

/**
 * @class RunAction
 * @brief Implementation of user run action hook
 */
class RunAction : public G4UserRunAction {
 public:
  /**
   * Class constructor.
   */
  RunAction();

  /**
   * Class destructor.
   */
  virtual ~RunAction();

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

};  // RunAction

}  // namespace g4user
}  // namespace simcore

#endif  // SIMCORE_G4USER_RUNACTION_H
