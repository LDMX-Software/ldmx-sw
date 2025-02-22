/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/PrimaryToEcalFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/PtrRetrieval.h"

namespace biasing {

PrimaryToEcalFilter::PrimaryToEcalFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");
}

void PrimaryToEcalFilter::stepping(const G4Step* step) {
  // Only process the primary electron track
  if (int parentID{step->GetTrack()->GetParentID()}; parentID != 0) return;

  if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted())
    return;

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's NOT in the calorimeter region
  auto current_region =
      step->GetTrack()->GetVolume()->GetLogicalVolume()->GetRegion();
  auto calorimeter_region =
      simcore::g4user::ptrretrieval::getRegion("CalorimeterRegion");
  if (!calorimeter_region) {
    ldmx_log(warn) << "Region 'CalorimeterRegion' not found in Geant4 region store";
  }
  if (current_region == calorimeter_region) return;

  // If the energy of the particle fell below threshold, stop processing the
  // event.
  if (auto energy{step->GetPostStepPoint()->GetTotalEnergy()};
      energy < threshold_) {
    /*
    std::cout << "[ PrimaryToEcalFilter ] : Aborting "
        <<
    G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() <<
    std::endl;
    */
    step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }
}

}  // namespace biasing

DECLARE_ACTION(biasing, PrimaryToEcalFilter)
