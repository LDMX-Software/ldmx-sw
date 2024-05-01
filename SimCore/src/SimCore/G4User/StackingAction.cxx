#include "SimCore/G4User/StackingAction.h"

namespace simcore {
namespace g4user {

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(
    const G4Track* track) {
  // Default value of a track is fUrgent.
  G4ClassificationOfNewTrack currentTrackClass =
      G4ClassificationOfNewTrack::fUrgent;

  // Get proposed new track classification from this plugin.
  for (auto& stackingAction : stackingActions_) {
    // Get proposed new track classification from this plugin.
    G4ClassificationOfNewTrack newTrackClass =
        stackingAction->ClassifyNewTrack(track, currentTrackClass);

    // Only set the current classification if the plugin changed it.
    if (newTrackClass != currentTrackClass) currentTrackClass = newTrackClass;
  }

  return currentTrackClass;
}

void StackingAction::NewStage() {
  for (auto& stackingAction : stackingActions_) stackingAction->NewStage();
}

void StackingAction::PrepareNewEvent() {
  for (auto& stackingAction : stackingActions_)
    stackingAction->PrepareNewEvent();
}

}  // namespace g4user
}  // namespace simcore
