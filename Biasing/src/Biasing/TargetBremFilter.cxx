
#include "Biasing/TargetBremFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackInformation.h"

namespace biasing {

TargetBremFilter::TargetBremFilter(const std::string& name,
                                   framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoilMaxPThreshold_ =
      parameters.getParameter<double>("recoil_max_p_threshold");
  bremEnergyThreshold_ =
      parameters.getParameter<double>("brem_min_energy_threshold");
  killRecoil_ = parameters.getParameter<bool>("kill_recoil_track");
}

TargetBremFilter::~TargetBremFilter() {}

G4ClassificationOfNewTrack TargetBremFilter::ClassifyNewTrack(
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

void TargetBremFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Only process the primary electron track
  if (track->GetParentID() != 0) return;

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11)
    return;

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the target region.
  if (auto region{
          track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("target") != 0)
    return;

  /*
  std::cout << "[TargetBremFilter] : Stepping primary electron in 'target'
  region into "
    << track->GetNextVolume()->GetName()
    << std::endl;
   */

  /**
   * Check if the electron will be exiting the target
   *
   * The 'recoil_PV' volume name is automatically constructed by Geant4's
   * GDML parser and was found by inspecting the geometry using a
   * visualization. This Physical Volume (PV) is associated with the
   * recoil parent volume and so it will break if the recoil parent volume
   * changes its name.
   *
   * We also check if the next volume is World_PV because in some geometries
   * (e.g. v14), there is a air-gap between the target region and the recoil.
   */
  if (auto volume{track->GetNextVolume()->GetName()};
      volume.compareTo("recoil_PV") == 0 or volume.compareTo("World_PV") == 0) {
    // If the recoil electron
    if (track->GetMomentum().mag() >= recoilMaxPThreshold_) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    // Get the electron secondries
    bool hasBremCandidate = false;
    if (auto secondaries = step->GetSecondary(); secondaries->size() == 0) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    } else {
      for (auto& secondary_track : *secondaries) {
        G4String processName =
            secondary_track->GetCreatorProcess()->GetProcessName();

        if (processName.compareTo("eBrem") == 0 &&
            secondary_track->GetKineticEnergy() > bremEnergyThreshold_) {
          auto trackInfo{simcore::UserTrackInformation::get(secondary_track)};
          trackInfo->tagBremCandidate();

          getEventInfo()->incBremCandidateCount();

          hasBremCandidate = true;
        }
      }
    }

    if (!hasBremCandidate) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    /*
    std::cout << "[TargetBremFilter] : Found brem candidate" << std::endl;
     */

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
}

void TargetBremFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing, TargetBremFilter)
