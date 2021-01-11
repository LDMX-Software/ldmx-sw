/**
 * @file USteppingAction.cxx
 * @author Omar Moreno, SLAC National Accelerator Laboraty
 */

#include "SimCore/USteppingAction.h"

namespace simcore {

void USteppingAction::UserSteppingAction(const G4Step* step) {
  for (auto& steppingAction : steppingActions_) steppingAction->stepping(step);
}

}  // namespace simcore
