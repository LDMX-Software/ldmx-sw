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
  if (auto pdgCh{track->GetParticleDefinition()->GetPDGCharge()};
      abs(pdgCh) == 0) {
    return;
  }

  // Only electrons in the Tagger region are of interest.
  auto current_region = (track->GetVolume()->GetLogicalVolume()->GetRegion());
  auto tagger_region = simcore::g4user::ptrretrieval::getRegion("tagger");
  if (!tagger_region) {
    ldmx_log(warn) << "Region 'tagger' not found in Geant4 region store";
  }
  if (current_region != tagger_region) return;

  // Check if we are exiting the tagger
  auto next_region = (track->GetNextVolume()->GetLogicalVolume()->GetRegion());
  if (next_region != tagger_region) {
    checkAbortEvent(track);
    return;
  }

  // A particle will only leave hits in the active silicon so other volumes can
  // be skipped for now.
  auto current_volume = (track->GetVolume());
  auto tagger_physical_volume =
      simcore::g4user::ptrretrieval::getPhysicalVolume("tagger_PV");
  if (!tagger_physical_volume) {
    ldmx_log(warn) << "Volume 'tagger_PV' not found in Geant4 volume store";
  }
  if (current_volume == tagger_physical_volume) return;

  // The copy number is used to identify which layer energy was deposited into.
  int copy_number{0};
  // Get the pre-step point
  auto* preStepPoint = step->GetPreStepPoint();
  if (preStepPoint) {
    // Get the touchable handle
    auto touchableHandle = preStepPoint->GetTouchableHandle();
    if (touchableHandle) {
      // Get the history
      auto* history = touchableHandle->GetHistory();
      if (history) {
        // Get the volume
        auto* volumeAtTwo = history->GetVolume(2);
        if (volumeAtTwo) {
          // Get the copy number
          copy_number = volumeAtTwo->GetCopyNo();
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

DECLARE_ACTION(biasing, TaggerHitFilter)
