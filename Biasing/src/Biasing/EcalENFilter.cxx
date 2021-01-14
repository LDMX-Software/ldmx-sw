
#include "Biasing/EcalENFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"

namespace ldmx {

EcalENFilter::EcalENFilter(const std::string& name, Parameters& parameters)
    : UserAction(name, parameters) {
  min_total_en_energy_ = parameters.getParameter<double>("min_total_en_energy");
}

void EcalENFilter::stepping(const G4Step* step) {
  if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted())
    return;
  if (step->GetTrack()->GetTrackID() != 1) return;

  // track is the primary electron and event hasn't been aborted yet
  auto start{step->GetPreStepPoint()};
  auto end{step->GetPostStepPoint()};
  if ((start->GetKineticEnergy() > min_total_en_energy_ and
       end->GetKineticEnergy() < min_total_en_energy_) or
      (start->GetPhysicalVolume()
               ->GetLogicalVolume()
               ->GetRegion()
               ->GetName()
               .compareTo("CalorimeterRegion") == 0 and
       end->GetPhysicalVolume()
               ->GetLogicalVolume()
               ->GetRegion()
               ->GetName()
               .compareTo("CalorimeterRegion") != 0)) {
    // we just stepped below the threshold or left the calorimeter region

    // Get the electron secondries
    auto secondaries = step->GetSecondary();
    /*
    std::cout << "[ EcalENFilter ] : Primary electron went below recoil energy
    threshold: "
        << "KE: " << step->GetTrack()->GetKineticEnergy() << "MeV "
        << "N Secondaries: " << secondaries->size() << std::endl;
    */

    if (!secondaries or secondaries->size() == 0) {
      /*
      std::cout << "[ EcalENFilter ] : "
          <<
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
          << " No secondaries at all. Aborting event..." << std::endl;
      */
      step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    double enEnergy(0.);
    for (auto& secondary_track : *secondaries) {
      G4String processName =
          secondary_track->GetCreatorProcess()->GetProcessName();

      /*
      std::cout << "[ EcalENFilter ] : "
          <<
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
          << " Secondary coming from process '"
          << processName << "': ";
      */

      if (processName.contains("electronNuclear")) {
        /*
        std::cout
            << " Found a secondary EN product with energy "
            << secondary_track->GetKineticEnergy() << " MeV.";
        */
        enEnergy += secondary_track->GetKineticEnergy();
      }  // check for hard recoil

      //std::cout << std::endl;
    }  // loop over secondaries

    if (enEnergy < min_total_en_energy_) {
      /*
      std::cout << "[ EcalENFilter ] : "
          <<
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
          << " Not enough energy went to EN secondaries. Aborting event..." <<
      std::endl;
      */
      step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
  }  // electron stepped below recoil energy threshold or left calorimeter
     // region
}

}  // namespace ldmx

DECLARE_ACTION(ldmx, EcalENFilter)
