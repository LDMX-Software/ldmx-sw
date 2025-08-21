#include "SimCore/G4User/StackingAction.h"

namespace simcore {
namespace g4user {

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(
    const G4Track* track) {
  // Default value of a track is fUrgent.
  G4ClassificationOfNewTrack current_track_class =
      G4ClassificationOfNewTrack::fUrgent;

  // Get proposed new track classification from this plugin.
  for (auto& stacking_action : stacking_actions_) {
    // Get proposed new track classification from this plugin.
    G4ClassificationOfNewTrack new_track_class =
        stacking_action->classifyNewTrack(track, current_track_class);

    // Only set the current classification if the plugin changed it.
    if (new_track_class != current_track_class) current_track_class = new_track_class;
  }

  return current_track_class;
}

void StackingAction::NewStage() {
  for (auto& stacking_action : stacking_actions_) stacking_action->newStage();
}

void StackingAction::PrepareNewEvent() {
  for (auto& stacking_action : stacking_actions_)
    stacking_action->prepareNewEvent();
}

}  // namespace g4user
}  // namespace simcore
