/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/TaggerHitFilter.h"

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"
#include "G4Step.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/PtrRetrieval.h"

namespace biasing {

TaggerHitFilter::TaggerHitFilter(const std::string& name,
                                 framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  layers_hit_ = parameters.getParameter<int>("layers_hit", 8);
  ldmx_log(debug) << " layers_hit_ = " << layers_hit_;
}

void TaggerHitFilter::stepping(const G4Step* step) {
  // The track associated with this step will allow for the extraction of info
  // needed to determine if this is the incident electron.
  auto track{step->GetTrack()};
  // Require that track is charged
  if (auto pdg_ch{track->GetParticleDefinition()->GetPDGCharge()};
      abs(pdg_ch) == 0) {
    return;
  }

  // Only electrons in the Tagger region are of interest.
  auto phys_vol{track->GetVolume()};
  auto volume{phys_vol ? phys_vol->GetLogicalVolume() : nullptr};
  auto current_region{volume ? volume->GetRegion() : nullptr};
  static auto tagger_region =
      simcore::g4user::ptrretrieval::getRegion("tagger");
  if (!tagger_region) {
    ldmx_log(warn) << "Region 'tagger' not found in Geant4 region store";
  }
  if (current_region != tagger_region) return;

  // Check if we are exiting the tagger
  auto next_phy_vol{track->GetNextVolume()};
  auto next_log_vol{next_phy_vol ? next_phy_vol->GetLogicalVolume() : nullptr};
  auto next_region{next_log_vol ? next_log_vol->GetRegion() : nullptr};
  if (next_region != tagger_region) {
    checkAbortEvent(track);
    return;
  }

  // A particle will only leave hits in the active silicon so other volumes can
  // be skipped for now.
  auto current_volume = (track->GetVolume());
  static auto tagger_physical_volume =
      simcore::g4user::ptrretrieval::getPhysicalVolume("tagger_PV");
  if (!tagger_physical_volume) {
    ldmx_log(warn) << "Volume 'tagger_PV' not found in Geant4 volume store";
  }
  if (current_volume == tagger_physical_volume) return;

  // The copy number is used to identify which layer energy was deposited into.
  int copy_number{0};
  // Get the pre-step point
  auto* pre_step_point = step->GetPreStepPoint();
  if (pre_step_point) {
    // Get the touchable handle
    const auto& touchable_handle = pre_step_point->GetTouchableHandle();
    if (touchable_handle) {
      // Get the history
      auto* history = touchable_handle->GetHistory();
      if (history) {
        // Get the volume
        auto* volume_at_two = history->GetVolume(2);
        if (volume_at_two) {
          // Get the copy number
          copy_number = volume_at_two->GetCopyNo();
        }
      }
    }
  }
  layer_count_.insert(copy_number);
}

void TaggerHitFilter::EndOfEventAction(const G4Event* event) {
  checkAbortEvent(nullptr);
  layer_count_.clear();
}

void TaggerHitFilter::checkAbortEvent(G4Track* track) {
  // These numbers may change in the future
  constexpr int early_layer_requirement = 10;
  constexpr int late_layer_requirement = 20;
  if ((layer_count_.size() < layers_hit_) ||
      ((layer_count_.count(early_layer_requirement) == 0) &&
       (layer_count_.count(late_layer_requirement) == 0))) {
    if (track != nullptr) track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }
}

}  // namespace biasing

DECLARE_ACTION(biasing::TaggerHitFilter)
