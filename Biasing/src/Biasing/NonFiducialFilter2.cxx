
#include "Biasing/NonFiducialFilter2.h"

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
#include "SimCore/UserTrackInformation.h"

namespace biasing {

NonFiducialFilter2::NonFiducialFilter2(const std::string& name,
                                     framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  process_ = parameters.getParameter<std::string>("process");
}

NonFiducialFilter2::~NonFiducialFilter2() {}

G4ClassificationOfNewTrack NonFiducialFilter2::ClassifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  // Get the particle type.
  G4String particleName = track->GetParticleDefinition()->GetParticleName();

  if (track == currentTrack_) {
    /*
    std::cout << "[ NonFiducialFilter2 ]: "
        << "Putting track " << track->GetTrackID()
        << " onto waiting stack." << std::endl;
    */
    currentTrack_ = nullptr;
    return fWaiting;
  }

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  return classification;
}

void NonFiducialFilter2::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted())
    return;

  // Get the track info and check if this track is a recoil electron 
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  if ((trackInfo != nullptr) && !trackInfo->isRecoilElectron()) return;

  // Get the region the particle is currently in. Abort the event if
  // the particle only is in the calorimeter region.
  if (auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("CalorimeterRegion") != 0) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        currentTrack_ = nullptr;
      }
}
}

DECLARE_ACTION(biasing, NonFiducialFilter2)
