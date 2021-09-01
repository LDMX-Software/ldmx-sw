
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

NonFiducialFilter::NonFiducialFilter(const std::string& name,
                                   framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoilMaxPThreshold_ =
      parameters.getParameter<double>("recoil_max_p_threshold");
  killRecoil_ = parameters.getParameter<bool>("kill_recoil_track");
}

NonFiducialFilter::~NonFiducialFilter() {}

G4ClassificationOfNewTrack NonFiducialFilter::ClassifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  // get the PDGID of the track.
  G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

  // Get the particle type.
  G4String particleName = track->GetParticleDefinition()->GetParticleName();

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  if (track->GetTrackID() == 1 && pdgID == 11) {
    return fWaiting;
  }

  return classification;
}

void NonFiducialFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Mark the event having no recoil electron initially
  bool hasRecoilElectron = false;

  // Only process the primary electron track
  if (track->GetParentID() != 0) return;

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11)
    return;

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the target region.
  if (auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("target") != 0)
    return;

  // Check if the electron will be exiting the target
  if (auto volume{track->GetNextVolume()->GetName()};
      volume.compareTo("recoil_PV") == 0) {
    // If the recoil electron exceeds the momentum threshold, kill the event.
    if (track->GetMomentum().mag() >= recoilMaxPThreshold_) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
  
    // Tag the event with having a recoil electron that satisfies entering/exiting the 
    // target and is the primary parent particle.
    auto trackInfo{simcore::UserTrackInformation::get(track)};
    trackInfo->tagRecoilElectron();
    hasRecoilElectron = true;

    // Check if the recoil electron should be killed.  If not, postpone
    // its processing until the brem gamma has been processed.
    if (killRecoil_)
      track->SetTrackStatus(fStopAndKill);
    else
      track->SetTrackStatus(fSuspend);

  } else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }

  if (!hasRecoilElectron) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
}

void NonFiducialFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing, NonFiducialFilter)
