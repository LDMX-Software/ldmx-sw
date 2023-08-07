#include "SimCore/G4User/SteppingAction.h"

namespace simcore::g4user {

void SteppingAction::UserSteppingAction(const G4Step* step) {
  auto event_info{static_cast<UserEventInformation*>(
      G4EventManager::GetEventManager()->GetUserInformation())};

  // get the track weights before this step and after this step
  //  ** these weights include the factors of all upstream step weights **
  double track_weight_pre_step = step->GetPreStepPoint()->GetWeight();
  double track_weight_post_step = step->GetPostStepPoint()->GetWeight();

  //  so, to get _this_ step's weight, we divide post_weight by pre_weight
  double weight_of_this_step_alone =
      track_weight_post_step / track_weight_pre_step;

  event_info->incWeight(weight_of_this_step_alone);

  const std::vector<const G4Track*>* secondaries{
      step->GetSecondaryInCurrentStep()};

  /**
   * Reset PN/EN flags and updating running energy totals
   *
   * The process of looping through the secondaries of *every*
   * step may slow down our simulation. I (Tom E) do not know how
   * much it would slow us down, but we could implement a procedure
   * to make this configurable if we wish.
   */
  event_info->lastStepWasPN(false);
  event_info->lastStepWasEN(false);
  if (secondaries) {
    double delta_energy = step->GetPreStepPoint()->GetKineticEnergy() -
                          step->GetPostStepPoint()->GetKineticEnergy();
    for (const G4Track* secondary : *secondaries) {
      const G4VProcess* creator{secondary->GetCreatorProcess()};
      if (creator) {
        const G4String& creator_name{creator->GetProcessName()};
        if (creator_name.contains("photonNuclear")) {
          event_info->addPNEnergy(delta_energy);
          event_info->lastStepWasPN(true);
          break;  // done <- assumes first match determines step process
        }
        if (creator_name.contains("electronNuclear")) {
          event_info->addENEnergy(delta_energy);
          event_info->lastStepWasEN(true);
          break;  // done <- assumes first match determines step process
        }         // creator name matches PN or EN
      }           // creator exists
    }             // loop over secondaries
  }               // secondaries list was created
  // now stepping actions can use getEventInfo()->wasLastStep{P,E}N()
  //  to determine if last step was PN or EN
  for (auto& steppingAction : steppingActions_) steppingAction->stepping(step);
}

}  // namespace simcore::g4user
