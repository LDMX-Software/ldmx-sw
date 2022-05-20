#pragma once

#include <algorithm>

#include "g4fire/UserAction.h"

#include "fire/config/Parameters.h"

namespace biasing {

/**
 * User action that allows a user to filter out events that don't result in
 * a brem within the target.
 */
class TargetBremFilter : public g4fire::UserAction {
public:
  /// Constructor
  TargetBremFilter(const std::string &name,
                   fire::config::Parameters &parameters);

  /// Destructor
  ~TargetBremFilter() = default;

  /**
   * Implement the stepping action which performs the target volume biasing.
   * @param step The Geant4 step.
   */
  void stepping(const G4Step *step) final override;

  /**
   * Method called at the end of every event.
   *
   * @param event Geant4 event object.
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
  /// Recoil electron threshold.
  double recoil_max_p_thresh_{1500}; // MeV

  /// Brem gamma energy treshold
  double brem_energy_thresh_{2500};

  /// Flag indicating if the recoil electron track should be killed
  bool kill_recoil_{false};

  /// Variable indicating whether a brem candidate has been found.
  bool brem_candidate_found_{false}; 

}; // TargetBremFilter
} // namespace biasing
