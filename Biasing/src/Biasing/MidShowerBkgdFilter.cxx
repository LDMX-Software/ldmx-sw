#include "Biasing/MidShowerBkgdFilter.h"

#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackInformation.h"

#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"

#define BIASING_MIDSHOWER_DEV_DEBUG

namespace ldmx {

MidShowerBkgdFilter::MidShowerBkgdFilter(const std::string& name,
                                         Parameters& parameters)
    : UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");
  process_ = parameters.getParameter<std::string>("process");
}

void MidShowerBkgdFilter::BeginOfEventAction(const G4Event*) {
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
  std::cout
      << "[ MidShowerBkgdFilter ]: "
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") "
      << "starting new simulation event." << std::endl;
#endif
  total_process_energy_ = 0.;
  track_weights_.clear();
}

void MidShowerBkgdFilter::stepping(const G4Step* step) {
  // skip steps that are outside the calorimeter region
  if (isOutsideCalorimeterRegion(step)) return;

  if (anyCreatedViaProcess(step->GetSecondaryInCurrentStep())) {
    double pre_energy = step->GetPreStepPoint()->GetTotalEnergy();
    double post_energy = step->GetPostStepPoint()->GetTotalEnergy();
    total_process_energy_ += pre_energy - post_energy;

    const G4Track* track = step->GetTrack();
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
    std::cout << "[ MidShowerBkgdFilter ]: "
              << "("
              << G4EventManager::GetEventManager()
                     ->GetConstCurrentEvent()
                     ->GetEventID()
              << ") "
              << "Track " << track->GetParentID() << " created "
              << track->GetTrackID() << " which went from " << pre_energy
              << " MeV to " << post_energy << " via " << process_ << std::endl;
#endif

    auto track_info =
        dynamic_cast<UserTrackInformation*>(track->GetUserInformation());

    // make sure this track is saved
    track_info->setSaveFlag(true);

    // record the weight of this track
    //  intentionally over-writing any previous storage of the weight
    //  this accounts for a particle that may undergo the desired process
    //  more than once
    track_weights_[track->GetTrackID()] = track->GetWeight();

#ifdef BIASING_MIDSHOWER_DEV_DEBUG
    std::cout << "[ MidShowerBkgdFilter ]: "
              << "("
              << G4EventManager::GetEventManager()
                     ->GetConstCurrentEvent()
                     ->GetEventID()
              << ") "
              << "resetting weight of track " << track->GetTrackID() << " to "
              << track->GetWeight() << std::endl;
#endif
  }  // there are interesting secondaries in this step
}

void MidShowerBkgdFilter::NewStage() {
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
  std::cout
      << "[ MidShowerBkgdFilter ]: "
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") " << total_process_energy_ << " MeV went " << process_ << std::endl;
#endif
  if (total_process_energy_ < threshold_)
    AbortEvent("Not enough energy went to the input process.");
  return;
}

void MidShowerBkgdFilter::EndOfEventAction(const G4Event* event) {
  // don't waste time if event is aborted
  if (event->IsAborted()) return;

  double event_weight{1.};
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
  std::cout << "[ MidShowerBkgdFilter ]: "
            << "(" << event->GetEventID() << ") "
            << "Weights:";
#endif
  for (auto const& [trackid, weight] : track_weights_) {
#ifdef BIASING_MIDSHOWER_DEV_DEBUG
    std::cout << " (" << trackid << "," << weight << ")";
#endif
    event_weight *= weight;
  }

#ifdef BIASING_MIDSHOWER_DEV_DEBUG
  std::cout << std::endl
            << "[ MidShowerBkgdFilter ]: "
            << "(" << event->GetEventID() << ") "
            << "Event weighted " << event_weight << std::endl;
#endif

  if (!event->GetUserInformation())
    const_cast<G4Event*>(event)->SetUserInformation(new UserEventInformation);

  auto event_info{
      static_cast<UserEventInformation*>(event->GetUserInformation())};
  event_info->setWeight(event_weight);
}

bool MidShowerBkgdFilter::isOutsideCalorimeterRegion(const G4Step* step) const {
  // the pointers in this chain are assumed to be always valid
  auto reg{step->GetTrack()->GetVolume()->GetLogicalVolume()->GetRegion()};
  if (reg) return (reg->GetName() != "CalorimeterRegion");
  // region is nullptr ==> no region defined for current volume
  //  ==> outside CalorimeterRegion
  return true;
}

bool MidShowerBkgdFilter::anyCreatedViaProcess(
    const std::vector<const G4Track*>* list) const {
  if (not list) return false;  // was list even created?
  for (auto const& track : *list) {
    const G4VProcess* creator = track->GetCreatorProcess();
    if (creator and creator->GetProcessName().contains(process_)) return true;
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
