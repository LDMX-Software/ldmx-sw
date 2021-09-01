#ifndef BIASING_NONFIDUCIALFILTER_H
#define BIASING_NONFIDUCIALFILTER_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

namespace biasing {

/**
 * User action that allows a user to filter out events that don't result in
 * a brem within the target.
 */
class NonFiducialFilter : public simcore::UserAction {
 public:
  /// Constructor
  NonFiducialFilter(const std::string& name,
                   framework::config::Parameters& parameters);

  /// Destructor
  ~NonFiducialFilter();

  /**
   * Implement the stepping action which performs the target volume biasing.
   * @param step The Geant4 step.
   */
  void stepping(const G4Step* step) final override;

  /**
   * Method called at the end of every event.
   *
   * @param event Geant4 event object.
   */
  void EndOfEventAction(const G4Event*) final override;

  /**
   * Classify a new track which postpones track processing.
   * Track processing resumes normally if a target PN interaction occurred.
   * @param aTrack The Geant4 track.
   * @param currentTrackClass The current track classification.
   */
  G4ClassificationOfNewTrack ClassifyNewTrack(
      const G4Track* aTrack,
      const G4ClassificationOfNewTrack& currentTrackClass) final override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::EVENT, simcore::TYPE::STACKING,
            simcore::TYPE::STEPPING};
  }

 private:
  /// Recoil electron threshold.
  double recoilMaxPThreshold_{1500};  // MeV

  /// Flag indicating if the recoil electron track should be killed
  bool killRecoil_{false};

};  // NonFiducialFilter
}  // namespace biasing

#endif  // BIASING_NONFIDUCIALFILTER_H
