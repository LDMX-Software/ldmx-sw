#include "Biasing/NonFiducialFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4String.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackInformation.h"

namespace biasing {

NonFiducialFilter::NonFiducialFilter(const std::string& name,framework::config::Parameters& parameters)
  : simcore::UserAction(name, parameters) {
  recoilMaxPThreshold_ =
      parameters.getParameter<double>("recoil_max_p_threshold");
      }

NonFiducialFilter::~NonFiducialFilter() {}

void NonFiducialFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Get the PDG ID of the track and make sure it's an electron.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11) {
    return;
  }

  // Check if the track is tagged.
  if (auto electronCheck{simcore::UserTrackInformation::get(track)}; electronCheck->isRecoilElectron() == true) { 
    // std::cout << "[ NonFiducialFilter ]: " << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << " is a tagged electron." << std::endl; 
    
    // Check if the track ever enters the ECal. If it does, kill the track and abort the event.
    if (auto volume{track->GetVolume()->GetLogicalVolume()->GetName()}; 
    (volume.contains("Si") || volume.contains("W") || volume.contains("PCB") || volume.contains("Readout") || volume.contains("CFMix") || volume.contains("Al")) 
    && volume.contains("volume")) {
      // std::cout << "[ NonFiducialFilter ]: " << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << " entered the ECal." << std::endl; 
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
    return;
  } else

  // Check if the particle enters the target.
  if (auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()}; region.compareTo("target") == 0) {
    // std::cout << "[NonFiducialFilter]: " << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << " entered the target." << std::endl;
    
    // Check if the particle that entered the target exits to the recoil region.
    if (auto next_region{track->GetNextVolume()->GetName()}; next_region.compareTo("recoil_PV") == 0) {
      // std::cout << "[NonFiducialFilter]: " << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << " exited the target." << std::endl;
      /* Tag the tracks that: 
      1) Have a recoil electron
      2) Enter/Exit the Target */
      auto trackInfo{simcore::UserTrackInformation::get(track)};
      trackInfo->tagRecoilElectron();
      // std::cout << "[ NonFiducialFilter ]: " << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << " tagged." << std::endl;
      return;
    }
    return;
  }
}
}

void NonFiducialFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing, NonFiducialFilter)