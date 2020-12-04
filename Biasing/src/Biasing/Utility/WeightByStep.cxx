#include "Biasing/Utility/WeightByStep.h"

#include "SimCore/UserEventInformation.h"

#include "G4EventManager.hh"
#include "G4Step.hh"

namespace biasing {
namespace utility {

WeightByStep::WeightByStep(const std::string& name, Parameters& parameters)
    : UserAction(name, parameters) {}

void WeightByStep::stepping(const G4Step* step) {
  // get the handle to the event user information
  //  this is where the event weight is stored
  auto event{G4EventManager::GetEventManager()};
  if (!event->GetUserInformation()) {
    // first step of the event ==> create user event info
    event->SetUserInformation(new UserEventInformation);
  }
  auto event_info{
      static_cast<UserEventInformation*>(event->GetUserInformation)};

  // get the track weights before this step and after this step
  //  ** these weights include the factors of all upstream step weights **
  double track_weight_pre_step = step->GetPreStepPoint()->GetWeight();
  double track_weight_post_step = step->GetPostStepPoint()->GetWeight();

  //  so, to get _this_ step's weight, we divide post_weight by pre_weight
  double weight_of_this_step_alone =
      track_weight_post_step / track_weight_pre_step;

  // factor this step's weight into the event weight
  event_info->incWeight(weight_of_this_step_alone);

  return;
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, WeightByStep)
