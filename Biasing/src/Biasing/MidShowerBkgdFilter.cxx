#include "Biasing/MidShowerBkgdFilter.h"

#include "SimCore/UserTrackInformation.h"

#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"

namespace ldmx {

MidShowerBkgdFilter::MidShowerBkgdFilter(const std::string& name,
                                       Parameters& parameters)
    : UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");
  process_   = parameters.getParameter<std::string>("process");
}

void MidShowerBkgdFilter::BeginOfEventAction(const G4Event*) {
  /* debug comment
  std::cout << "[ MidShowerBkgdFilter ]: "
    << "("
    << G4EventManager::GetEventManager()
           ->GetConstCurrentEvent()
           ->GetEventID()
    << ") "
    << "starting new simulation event."
    << std::endl;
   */
  total_process_energy_ = 0.;
}

void MidShowerBkgdFilter::stepping(const G4Step* step) {

  //skip steps that are outside the calorimeter region
  if (isOutsideCalorimeterRegion(step)) return;

  if (anyCreatedViaProcess(step->GetSecondaryInCurrentStep())) {
    double pre_energy = step->GetPreStepPoint()->GetTotalEnergy();
    double post_energy= step->GetPostStepPoint()->GetTotalEnergy();
    /* debug comment
    std::cout << "[ MidShowerBkgdFilter ]: "
      << "("
      << G4EventManager::GetEventManager()
             ->GetConstCurrentEvent()
             ->GetEventID()
      << ") "
      << "Track " << step->GetTrack()->GetParentID()
      << " created " << step->GetTrack()->GetTrackID()
      << " which went from " << pre_energy << " MeV to " 
      << post_energy << " via " << process_
      << std::endl;
     */
    total_process_energy_ += pre_energy - post_energy;

    auto track_info = dynamic_cast<UserTrackInformation*>(
        step->GetTrack()->GetUserInformation());

    track_info->setSaveFlag(true);
  } // there are interesting secondaries in this step
}

void MidShowerBkgdFilter::NewStage() {
  /* debug comment
  std::cout << "[ MidShowerBkgdFilter ]: "
    << "("
    << G4EventManager::GetEventManager()
           ->GetConstCurrentEvent()
           ->GetEventID()
    << ") "
    << total_process_energy_ << " MeV went " << process_
    << std::endl;
   */

  if (total_process_energy_ < threshold_) AbortEvent("Not enough energy went to the input process.");
  return;
}

bool MidShowerBkgdFilter::isOutsideCalorimeterRegion(const G4Step* step) const {
  //the pointers in this chain are assumed to be always valid
  auto reg{step->GetTrack()->GetVolume()->GetLogicalVolume()->GetRegion()};
  if (reg) return (reg->GetName() != "CalorimeterRegion");
  //region is nullptr ==> no region defined for current volume
  //  ==> outside CalorimeterRegion
  return true;
}

bool MidShowerBkgdFilter::anyCreatedViaProcess(const std::vector<const G4Track*>* list) const {
  if (not list) return false; //was list even created?
  for (auto const& track : *list ) {
    const G4VProcess* creator = track->GetCreatorProcess();
    if (creator and creator->GetProcessName().contains(process_))
      return true;
  }
  return false;
}

void MidShowerBkgdFilter::AbortEvent(const std::string& reason) const {
  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
    std::cout << "[ MidShowerBkgdFilter ]: "
              << "("
              << G4EventManager::GetEventManager()
                     ->GetConstCurrentEvent()
                     ->GetEventID()
              << ") " << reason << " Aborting event." << std::endl;
  }
  G4RunManager::GetRunManager()->AbortEvent();
  return;
}
}  // namespace ldmx

DECLARE_ACTION(ldmx, MidShowerBkgdFilter)
