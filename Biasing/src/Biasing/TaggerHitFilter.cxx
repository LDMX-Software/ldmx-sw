
#include "Biasing/TaggerHitFilter.h"

//~~ Geant4 ~~//
#include "G4RunManager.hh"
#include "G4Step.hh"

namespace biasing {

TaggerHitFilter::TaggerHitFilter(const std::string& name,
                                 framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  layers_hit_ = parameters.getParameter<int>("layers_hit", 8);
}

void TaggerHitFilter::stepping(const G4Step* step) {
  // The track associated with this step will allow for the extraction of info
  // needed to determine if this is the incident electron.
  auto track{step->GetTrack()};

  // Only process the primary electron i.e. a particle without parents.
  if (track->GetParentID() != 0) return;

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception because the generator
  // being used is likely wrong.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11)
    return;

  // Only electrons in the Tagger region are of interest.
  auto volume{track->GetVolume()};
  if (auto region{volume->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("tagger") != 0)
    return;

  // Check if the number of layers hit is above the threshold if 
  // 1) the incident electron has exited the tagger tracker volume
  // 2) the incident electron has lost all of its energy.
  // If the number of sensors hit is below the layer threshold, abort the 
  // event.
  if (auto nregion{track->GetNextVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      (nregion.compareTo("tagger") != 0) ||
      (track->GetKineticEnergy() == 0)) {
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

  // Use a set to keep track of the number of unique layers with energy
  // depositions.
  layer_count_.insert(copy_number);
}

void TaggerHitFilter::EndOfEventAction(const G4Event* event) {
  checkAbortEvent(nullptr); 
  layer_count_.clear();
}

void TaggerHitFilter::checkAbortEvent(G4Track* track) { 
    if ((layer_count_.size() < layers_hit_) 
        || ((layer_count_.count(10) == 0) 
          && (layer_count_.count(20) == 0))) {
      if (track != nullptr) track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      //std::cout << "Aborting" << std::endl;
      return;
    }
    std::cout << "[ "; 
    for (auto &l : layer_count_) std::cout << l << ", "; 
    std::cout << " ]" << std::endl;
}

}  // namespace biasing

DECLARE_ACTION(biasing, TaggerHitFilter)
