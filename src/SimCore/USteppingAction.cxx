/**
 * @file USteppingAction.cxx
 * @author Omar Moreno, SLAC National Accelerator Laboraty
 */

#include "SimCore/USteppingAction.h"

namespace ldmx {

void USteppingAction::UserSteppingAction(const G4Step* step) {
  for (auto& steppingAction : steppingActions_) steppingAction->stepping(step);
}

}  // namespace ldmx
