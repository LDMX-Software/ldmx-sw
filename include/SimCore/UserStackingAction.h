/**
 * @file UserStackingAction.h
 * @brief Class which implements the Geant4 user stacking action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USERSTACKINGACTION_H
#define SIMCORE_USERSTACKINGACTION_H

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
#include "SimCore/UserAction.h"

namespace simcore {

/**
 * @class UserStackingAction
 * @brief Class implementing a user stacking action.
 */
class UserStackingAction : public G4UserStackingAction {
 public:
  /// Constructor
  UserStackingAction();

  /// Destructor
  virtual ~UserStackingAction() final override;

  /**
   * Classify a new track.
   * @param aTrack The track to classify.
   * @return The track classification.
   */
  G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* track);

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
  void registerAction(UserAction* stackingAction) {
    stackingActions_.push_back(stackingAction);
  }

 private:
  /// Collection of user stacking actions
  std::vector<UserAction*> stackingActions_;

};  // UserStackingAction

}  // namespace simcore

#endif  // SIMCORE_USERSTACKINGACTION_H
