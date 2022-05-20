#pragma once


#include "g4fire/UserAction.h"

#include "fire/config/Parameters.h"

namespace biasing {

/**
 * @class TargetProcessFilter
 * @brief Biases Geant4 to only process events where PN reaction occurred in the
 * target
 */
class TargetProcessFilter : public g4fire::UserAction {
 public:
  /**
   * Class constructor.
   */
  TargetProcessFilter(const std::string &name,
                      fire::config::Parameters &parameters);

  /// Destructor
  ~TargetProcessFilter() = default;

  /**
   * Implementmthe stepping action which performs the target volume biasing.
   * @param step The Geant4 step.
   */
  void stepping(const G4Step *step) final override;

  /**
   * End of event action.
   */
  void EndOfEventAction(const G4Event *) final override;

  /**
   * Classify a new track which postpones track processing.
   * Track processing resumes normally if a target PN interaction occurred.
   * @param track The Geant4 track.
   * @param track_class The current track classification.
   */
  G4ClassificationOfNewTrack ClassifyNewTrack(
      const G4Track *track,
      const G4ClassificationOfNewTrack &track_class) final override;

  /// Retrieve the type of actions this class defines
  std::vector<g4fire::TYPE> getTypes() final override {
    return {g4fire::TYPE::EVENT, g4fire::TYPE::STACKING,
            g4fire::TYPE::STEPPING};
  }

 private:
  /** Pointer to the current track being processed. */
  G4Track *ctrack_{nullptr};

  /** Flag indicating if the reaction of intereset occurred. */
  bool reaction_occurred_{false};

  /// The process to bias
  std::string process_{""};

}; // TargetProcessFilter  
}  // namespace biasing
