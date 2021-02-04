#include "SimCore/UserStackingAction.h"

namespace simcore {

UserStackingAction::UserStackingAction() {}

UserStackingAction::~UserStackingAction() {}

G4ClassificationOfNewTrack UserStackingAction::ClassifyNewTrack(
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

void UserStackingAction::NewStage() {
  for (auto& stackingAction : stackingActions_) stackingAction->NewStage();
}

void UserStackingAction::PrepareNewEvent() {
  for (auto& stackingAction : stackingActions_)
    stackingAction->PrepareNewEvent();
}
}  // namespace simcore
