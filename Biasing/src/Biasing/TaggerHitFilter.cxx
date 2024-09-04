
#include "Biasing/TaggerHitFilter.h"

//~~ Geant4 ~~//
#include "G4RunManager.hh"
#include "G4Step.hh"

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
  auto volume{track->GetVolume()};
  if (auto region{volume->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("tagger") != 0)
    return;

  // Check if we are exiting the tagger
  if (auto nregion{
          track->GetNextVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      (nregion.compareTo("tagger") != 0)) {
    checkAbortEvent(track);
    return;
  }

  // A particle will only leave hits in the active silicon so other volumes can
  // be skipped for now.
  if (auto volume_name{track->GetVolume()->GetName()};
      volume_name.compareTo("tagger_PV") == 0)
    return;

  // The copy number is used to identify which layer energy was deposited into.
  auto copy_number{step->GetPreStepPoint()
                       ->GetTouchableHandle()
                       ->GetHistory()
                       ->GetVolume(2)
                       ->GetCopyNo()};

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
