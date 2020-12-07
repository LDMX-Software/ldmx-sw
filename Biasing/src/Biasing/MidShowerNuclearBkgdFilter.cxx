#include "Biasing/MidShowerNuclearBkgdFilter.h"

#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackInformation.h"

#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"

#define BIASING_MIDSHOWER_DEV_DEBUG

namespace ldmx {

MidShowerNuclearBkgdFilter::MidShowerNuclearBkgdFilter(const std::string& name,
                                         Parameters& parameters)
    : UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");
  nuclear_processes_ = { "photonNuclear" , "electronNuclear" };
}

void MidShowerNuclearBkgdFilter::BeginOfEventAction(const G4Event*) {
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
  std::cout
      << "[ MidShowerNuclearBkgdFilter ]: "
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") "
      << "starting new simulation event." << std::endl;
#endif
  total_process_energy_ = 0.;
}

void MidShowerNuclearBkgdFilter::stepping(const G4Step* step) {
  // skip steps that are outside the calorimeter region
  if (isOutsideCalorimeterRegion(step)) return;

  if (anyCreatedViaNuclear(step->GetSecondaryInCurrentStep())) {
    double pre_energy = step->GetPreStepPoint()->GetTotalEnergy();
    double post_energy = step->GetPostStepPoint()->GetTotalEnergy();
    total_process_energy_ += pre_energy - post_energy;

    const G4Track* track = step->GetTrack();
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
    std::cout << "[ MidShowerNuclearBkgdFilter ]: "
              << "("
              << G4EventManager::GetEventManager()
                     ->GetConstCurrentEvent()
                     ->GetEventID()
              << ") "
              << "Track " << track->GetParentID() << " created "
              << track->GetTrackID() << " which went from " << pre_energy
              << " MeV to " << post_energy << " via a nuclear process." << std::endl;
#endif

    auto track_info =
        dynamic_cast<UserTrackInformation*>(track->GetUserInformation());

    // make sure this track is saved
    track_info->setSaveFlag(true);
  }  // there are interesting secondaries in this step
}

void MidShowerNuclearBkgdFilter::NewStage() {
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
  std::cout
      << "[ MidShowerNuclearBkgdFilter ]: "
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") " << total_process_energy_ << " MeV went nuclear." << std::endl;
#endif
  if (total_process_energy_ < threshold_)
    AbortEvent("Not enough energy went to the input process.");
  return;
}

bool MidShowerNuclearBkgdFilter::isOutsideCalorimeterRegion(const G4Step* step) const {
  // the pointers in this chain are assumed to be always valid
  auto reg{step->GetTrack()->GetVolume()->GetLogicalVolume()->GetRegion()};
  if (reg) return (reg->GetName() != "CalorimeterRegion");
  // region is nullptr ==> no region defined for current volume
  //  ==> outside CalorimeterRegion
  return true;
}

bool MidShowerNuclearBkgdFilter::anyCreatedViaNuclear(
    const std::vector<const G4Track*>* list) const {
  if (not list) return false;  // was list even created?
  for (auto const& track : *list) {
    const G4VProcess* creator = track->GetCreatorProcess();
    if (creator) {
      for (auto const& proc : nuclear_processes_) {
        if (creator->GetProcessName().contains(proc)) 
          return true;
      } //loop over nuclear processes
    } //creator process exists (i.e. track is not primary)
  }
  return false;
}

void MidShowerNuclearBkgdFilter::AbortEvent(const std::string& reason) const {
  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
    std::cout << "[ MidShowerNuclearBkgdFilter ]: "
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

DECLARE_ACTION(ldmx, MidShowerNuclearBkgdFilter)
