#ifndef BIASING_ECALPROCESSFILTER_H
#define BIASING_ECALPROCESSFILTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

// Forward declaration
class G4Step;
class G4Track;

namespace biasing {

/**
 * User action plugin that filters events that don't see a hard brem from
 * the target undergo a photo-nuclear reaction in the ECal.
 */
class EcalProcessFilter : public simcore::UserAction {
 public:
  /**
   *
   */
  EcalProcessFilter(const std::string& name,
                    framework::config::Parameters& parameters);

  /// Destructor
  virtual ~EcalProcessFilter() = default;

  void stepping(const G4Step* step) override;

  // void PostUserTrackingAction(const G4Track*) override;

  /**
   * Classify a new track which postpones track processing.
   * Track processing resumes normally if a target PN interaction occurred.
   * @param aTrack The Geant4 track.
   * @param currentTrackClass The current track classification.
   */
  G4ClassificationOfNewTrack classifyNewTrack(
      const G4Track* aTrack,
      const G4ClassificationOfNewTrack& currentTrackClass) override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() override {
    return {simcore::TYPE::STACKING, simcore::TYPE::STEPPING};
  }

 private:
  /** Pointer to the current track being processed. */
  G4Track* current_track_{nullptr};

  /// Process to filter
  std::string process_{""};

};  // EcalProcessFilter
}  // namespace biasing

#endif  // BIASING_ECALPROCESSFILTER_H
