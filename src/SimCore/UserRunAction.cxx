/**
 * @file UserRunAction.cxx
 * @brief Class which implements user run action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/UserRunAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Run.hh"

namespace simcore {

UserRunAction::UserRunAction() {}

UserRunAction::~UserRunAction() {}

void UserRunAction::BeginOfRunAction(const G4Run* run) {
  // Call user run action
  for (auto& runAction : runActions_) runAction->BeginOfRunAction(run);
}

void UserRunAction::EndOfRunAction(const G4Run* run) {
  // Call user run action
  for (auto& runAction : runActions_) runAction->EndOfRunAction(run);
}

}  // namespace simcore
