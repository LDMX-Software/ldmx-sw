
#include "Biasing/TaggerHitFilter.h"

//~~ Geant4 ~~//
#include "G4RunManager.hh"
#include "G4Step.hh"

namespace biasing {

TaggerHitFilter::TaggerHitFilter(const std::string& name,
                                 framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  layers_hit_ = parameters.getParameter<int>("layers_hit", 8);
  std::cout << "TaggerHitFilter:: layers_hit_ = " << layers_hit_ << std::endl;
}

void TaggerHitFilter::stepping(const G4Step* step) {
  // The track associated with this step will allow for the extraction of info
  // needed to determine if this is the incident electron.
  auto track{step->GetTrack()};
  //  std::cout<<"TaggeHitFilter::stepping"<<std::endl;
  // Only process the primary electron i.e. a particle without parents.
  //  June 21, 2024 MG:  Not sure this is a good cut to make here..
  //                     What about secondaries?
  //  if (track->GetParentID() != 0) return;

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception because the generator
  // being used is likely wrong.
  //  std::cout<<"Track pdgID =
  //  "<<track->GetParticleDefinition()->GetPDGEncoding()<<std::endl; June 21,
  //  2024 MG:  similar here, though it's unlikely to be aything else if (auto
  //  pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11){
  //  return;
  //}
  // Require that track is charged
  if (auto pdgCh{track->GetParticleDefinition()->GetPDGCharge()};
      abs(pdgCh) != 1) {
    return;
  }

  // Only electrons in the Tagger region are of interest.
  auto volume{track->GetVolume()};
  //  std::cout<<"Volume Name =
  //  "<<volume->GetLogicalVolume()->GetRegion()->GetName()<<std::endl;
  if (auto region{volume->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("tagger") != 0)
    return;

  // Check if the number of layers hit is above the threshold if
  // 1) the incident electron has exited the tagger tracker volume
  // 2) the incident electron has lost all of its energy.
  // If the number of sensors hit is below the layer threshold, abort the
  // event.
  //  std::cout<<"Next Volume Name =
  //  "+track->GetNextVolume()->GetLogicalVolume()->GetRegion()->GetName()<<std::endl;
  /*
  if (auto
  nregion{track->GetNextVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      (nregion.compareTo("tagger") != 0) ||
      (track->GetKineticEnergy() == 0)) {
      checkAbortEvent(track);
      return;
  }
  */
  // MG:  remove the kinetic energy requirement
  //      this basically just checks if we are exiting the tagger
  if (auto nregion{
          track->GetNextVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      (nregion.compareTo("tagger") != 0)) {
    checkAbortEvent(track);
    return;
  }

  // A particle will only leave hits in the active silicon so other volumes can
  // be skipped for now.
  //  std::cout<<"check active Si volume
  //  "<<track->GetVolume()->GetName()<<std::endl;
  if (auto volume_name{track->GetVolume()->GetName()};
      volume_name.compareTo("tagger_PV") == 0)
    return;

  // The copy number is used to identify which layer energy was deposited into.
  auto copy_number{step->GetPreStepPoint()
                       ->GetTouchableHandle()
                       ->GetHistory()
                       ->GetVolume(2)
                       ->GetCopyNo()};
  //  std::cout<<"putting copy_number "<<copy_number<<std::endl;
  // Use a set to keep track of the number of unique layers with energy
  // depositions.

  // MG:  only count front layers of tracker if from beam electron
  // if layers 1 or 2 (copy_number = 10 or 20) count
  // from any source
  //  if (copy_number<21){
  //  std::cout<<"Found inner layer hit "<<copy_number<<"  Parent ID  "<<
  //  track->GetParentID()<<std::endl;
  //}
  //  if (copy_number>20 && track->GetParentID() != 0)
  //  return;
  //  std::cout<<"Inserting hit "<<copy_number<<"  Parent ID  "<<
  //  track->GetParentID()<<std::endl;
  layer_count_.insert(copy_number);
}

void TaggerHitFilter::EndOfEventAction(const G4Event* event) {
  checkAbortEvent(nullptr);
  layer_count_.clear();
}

void TaggerHitFilter::checkAbortEvent(G4Track* track) {
  if ((layer_count_.size() < layers_hit_) ||
      ((layer_count_.count(10) == 0) && (layer_count_.count(20) == 0))) {
    //  if (layer_count_.size() < layers_hit_) {
    if (track != nullptr) track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    /*
          std::cout << "Aborting" << std::endl;
          std::cout<<"layer_count = "<<layer_count_.size()<<std::endl;
          std::cout<<"layer_count_.count(10) "<<layer_count_.count(10) <<
       std::endl; std::cout<<"layer_count_.count(20) "<<layer_count_.count(20)
       << std::endl;
    */
    return;
  }

  /*
  std::cout << "[ ";
  for (auto &l : layer_count_) std::cout << l << ", ";
  std::cout << " ]" << std::endl;
  */
}

}  // namespace biasing

DECLARE_ACTION(biasing, TaggerHitFilter)
