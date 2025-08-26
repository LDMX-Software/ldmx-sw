/**
 * @file USteppingAction.h
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_G4USER_STEPPINGACTION_H
#define SIMCORE_G4USER_STEPPINGACTION_H

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
#include "SimCore/G4User/UserAction.h"

namespace simcore {
namespace g4user {

/**
 * @class SteppingAction
 * @brief Implements the Geant4 user stepping action.
 */
class SteppingAction : public G4UserSteppingAction {
 public:
  /// Destructor
  ~SteppingAction() {}

  /**
   * Callback used to process a step.
   *
   * @param step The Geant4 step.
   */
  void UserSteppingAction(const G4Step* step) override;

  /**
   * Register a user action of type SteppingAction with this class.
   *
   * @param action  User action of type SteppingAction
   */
  void registerAction(std::shared_ptr<UserAction> steppingAction) {
    stepping_actions_.push_back(steppingAction);
  }

 private:
  /// Collection of user stepping actions
  std::vector<std::shared_ptr<UserAction>> stepping_actions_;

};  // SteppingAction

}  // namespace g4user
}  // namespace simcore

#endif  // SIMCORE_G4USER_STEPPINGACTION_H
