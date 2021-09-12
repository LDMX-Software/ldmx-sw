#include "Biasing/NonFiducialFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"

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

// Sets the amount of recoil electrons found to 0 for the beginning of each event.
void NonFiducialFilter::BeginOfEventAction(const G4Event*){
foundRecoilElectron_ = false;
return;
}

void NonFiducialFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11)
    return;
  
  // Check if the particle is tagged as a recoil electron.
  if (auto electronCheck{simcore::UserTrackInformation::get(track)}; electronCheck->isRecoilElectron() == true)
  {
    // Check if the recoil electron enters inside any ECal volume. If it does, kill the track.
    if (auto vol{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()}; 
    (vol.contains("Si") || vol.contains("W") || vol.contains("PCB") || vol.contains("Readout") || vol.contains("CFMix") || vol.contains("Al")) && vol.contains("volume"))
    {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
  }
  // Check if the particle is in the Target.
  else if (auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()}; region.compareTo("target") == 0)
  {
  // Check if the electron will be exiting the Target.
  if (auto volume{track->GetNextVolume()->GetName()}; volume.compareTo("recoil") == 0) 
  {
    // If the recoil electron exceeds the momentum threshold, kill the track.
    if (track->GetMomentum().mag() >= recoilMaxPThreshold_) 
    {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
    
    // If it doesn't exit the Target and the kinetic energy is 0, kill the track.
    else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) 
    {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
    }

  // Tag the tracks that: 
  // 1) Have a recoil electron
  // 2) Enter/Exit the Target
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  trackInfo->tagRecoilElectron();
  // Abort multi-electron events.
  if (foundRecoilElectron_) {
  G4RunManager::GetRunManager()->AbortEvent();
  } else {
  foundRecoilElectron_ = true;  
  }
  }
  }
}

void NonFiducialFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing, NonFiducialFilter)
