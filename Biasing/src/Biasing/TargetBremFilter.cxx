
#include "Biasing/TargetBremFilter.h"

#include "G4EventManager.hh"
#include "G4RunManager.hh"

#include "g4fire/UserEventInformation.h"
#include "g4fire/UserTrackInformation.h"

#include "fire/exception/Exception.h"

namespace biasing {

TargetBremFilter::TargetBremFilter(const std::string &name,
                                   fire::config::Parameters &parameters)
    : g4fire::UserAction(name, parameters) {

  // Set the parameters
  recoil_max_p_thresh_ = parameters.get<double>("recoil_max_p_threshold");
  brem_energy_thresh_ = parameters.get<double>("brem_min_energy_threshold");
  kill_recoil_ = parameters.get<bool>("kill_recoil_track");
}

G4ClassificationOfNewTrack TargetBremFilter::ClassifyNewTrack(
    const G4Track *track, const G4ClassificationOfNewTrack &track_class) {

  // get the PDGID of the track.
  auto pdg_id{track->GetParticleDefinition()->GetPDGEncoding()};

  // Get the particle type.
  auto particle_name{track->GetParticleDefinition()->GetParticleName()};

  if (track->GetTrackID() == 1 && pdg_id == 11) {
    return fWaiting;
  }

  // Use current classification by default so values from other plugins are not
  // overridden.
  return track_class;
}

void TargetBremFilter::stepping(const G4Step *step) {
  if (brem_candidate_found_)
    return;

  // Get the track associated with this step.
  auto track{step->GetTrack()};
  // Only process the primary electron track
  if (track->GetParentID() != 0) {
    return;
  }

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception.
  if (auto pdg_id{track->GetParticleDefinition()->GetPDGEncoding()};
      pdg_id != 11)
    throw fire::Exception("Fatal",
                          "Primary particle is not an electron. Please check "
                          "the particle gun configuration.",
                          false);

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the target region.
  if (auto region{
          track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("target") != 0) {
    return;
  }
  // Check if the electron will be exiting the target
  if (auto nregion{
          track->GetNextVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      nregion.compareTo("target") != 0) {
    // If the recoil electron
    if (track->GetMomentum().mag() >= recoil_max_p_thresh_) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    // Get the electron secondries
    if (auto secondaries = step->GetSecondary(); secondaries->size() == 0) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    } else {
      for (auto &secondary_track : *secondaries) {
        G4String processName =
            secondary_track->GetCreatorProcess()->GetProcessName();
        if (processName.compareTo("eBrem") == 0 &&
            secondary_track->GetKineticEnergy() > brem_energy_thresh_) {
          auto trackInfo{g4fire::UserTrackInformation::get(secondary_track)};
          trackInfo->tagBremCandidate();

          getEventInfo()->incBremCandidateCount();
          brem_candidate_found_ = true;
        }
      }
    }

    if (!brem_candidate_found_) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    // Check if the recoil electron should be killed.  If not, postpone
    // its processing until the brem gamma has been processed.
    if (kill_recoil_)
      track->SetTrackStatus(fStopAndKill);
    else
      track->SetTrackStatus(fSuspend);

  } else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }
}

void TargetBremFilter::EndOfEventAction(const G4Event *) {
  brem_candidate_found_ = false;
}
} // namespace biasing

DECLARE_ACTION(biasing, TargetBremFilter)
