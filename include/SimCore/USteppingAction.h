/**
 * @file USteppingAction.h
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USTEPPINGACTION_H
#define SIMCORE_USTEPPINGACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <vector>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4UserSteppingAction.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

namespace simcore {

/**
 * @class USteppingAction
 * @brief Implements the Geant4 user stepping action.
 */
class USteppingAction : public G4UserSteppingAction {
 public:
  /// Destructor
  ~USteppingAction() final override { ; }

  /**
   * Callback used to process a step.
   *
   * @param step The Geant4 step.
   */
  void UserSteppingAction(const G4Step* step) final override;

  /**
   * Register a user action of type SteppingAction with this class.
   *
   * @param action  User action of type SteppingAction
   */
  void registerAction(UserAction* steppingAction) {
    steppingActions_.push_back(steppingAction);
  }

 private:
  /// Collection of user stepping actions
  std::vector<UserAction*> steppingActions_;

};  // USteppingAction

}  // namespace simcore

#endif  // SIMCORE_USTEPPINGACTION_H
