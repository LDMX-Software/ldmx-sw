/**
 * @file UserRunAction.cxx
 * @brief Class which implements user run action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/G4User/RunAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Run.hh"

namespace simcore {
namespace g4user {

void RunAction::BeginOfRunAction(const G4Run* run) {
  // Call user run action
  for (auto& run_action : run_actions_) {
    run_action->beginOfRunAction(run);
  }
}

void RunAction::EndOfRunAction(const G4Run* run) {
  // Call user run action
  for (auto& run_action : run_actions_) {
    run_action->endOfRunAction(run);
  }
}

}  // namespace g4user
}  // namespace simcore
