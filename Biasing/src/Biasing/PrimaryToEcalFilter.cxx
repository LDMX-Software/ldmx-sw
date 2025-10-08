#include "Biasing/PrimaryToEcalFilter.h"

namespace biasing {

PrimaryToEcalFilter::PrimaryToEcalFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  threshold_ = parameters.get<double>("threshold");
}

void PrimaryToEcalFilter::stepping(const G4Step* step) {
  // Only process the primary electron track
  if (int parent_id{step->GetTrack()->GetParentID()}; parent_id != 0) return;

  if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted())
    return;

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's NOT in the calorimeter region
  auto current_region =
      step->GetTrack()->GetVolume()->GetLogicalVolume()->GetRegion();
  auto calorimeter_region =
      simcore::g4user::ptrretrieval::getRegion("CalorimeterRegion");
  if (!calorimeter_region) {
    ldmx_log(warn)
        << "Region 'CalorimeterRegion' not found in Geant4 region store";
  }
  if (current_region == calorimeter_region) return;

  // If the energy of the particle fell below threshold, stop processing the
  // event.
  if (auto energy{step->GetPostStepPoint()->GetTotalEnergy()};
      energy < threshold_) {
    ldmx_log(trace) << "Aborting "
                    << G4EventManager::GetEventManager()
                           ->GetConstCurrentEvent()
                           ->GetEventID();

    step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }
}

}  // namespace biasing

DECLARE_ACTION(biasing::PrimaryToEcalFilter)
