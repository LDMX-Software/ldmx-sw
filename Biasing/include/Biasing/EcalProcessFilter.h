#ifndef BIASING_ECALPROCESSFILTER_H
#define BIASING_ECALPROCESSFILTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

// Forward declaration
class G4Step;
class G4Track;
#include "G4Gamma.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4ProcessTable.hh"
#include "G4RegionStore.hh"
#include "G4LogicalVolume.hh"

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
  ~EcalProcessFilter();

  void stepping(const G4Step* step) final override;

  // void PostUserTrackingAction(const G4Track*) final override;

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
    return {simcore::TYPE::STACKING, simcore::TYPE::STEPPING};
  }

 private:
  /** Pointer to the current track being processed. */
  G4Track* currentTrack_{nullptr};

  /// Process to filter
  std::string process_{""};

  /// Enable logging
  enableLogging("EcalProcessFilter")

      G4Region* region_;
  G4VPhysicalVolume* hcal_pv_;
  G4LogicalVolume* hcal_lv_;
  G4VProcess* processPtr_;
    G4bool IsHcalVolume(G4VPhysicalVolume * vol) {
      return hcal_lv_->IsAncestor(vol);
    }

};  // EcalProcessFilter
}  // namespace biasing

#endif  // BIASING_ECALPROCESSFILTER_H
