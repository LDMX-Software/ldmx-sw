
#include "Biasing/TargetProcessFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"

/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/TargetBremFilter.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"

namespace biasing {

TargetProcessFilter::TargetProcessFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  process_ = parameters.getParameter<std::string>("process");
}

TargetProcessFilter::~TargetProcessFilter() {}

G4ClassificationOfNewTrack TargetProcessFilter::ClassifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  if (track == currentTrack_) {
    currentTrack_ = nullptr;
    // std::cout << "[ TargetBremFilter ]: Pushing track to waiting stack." <<
    // std::endl;
    return fWaiting;
  }

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  return classification;
}

void TargetProcessFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted())
    return;

  // Get the track info and check if this track is a brem candidate
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  if ((trackInfo != nullptr) && !trackInfo->isBremCandidate()) return;

  // Get the particles daughters.
  auto secondaries{step->GetSecondary()};

  // Get the region the particle is currently in. Continue processing
  // the particle only if it's in the target region.
  if (auto region{
          track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("target") != 0) {
    // If secondaries were produced outside of the volume of interest,
    // and there aren't additional brems to process, abort the event.
    // Otherwise, suspend the track and move on to the next brem.
    if (secondaries->size() != 0) {
      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        currentTrack_ = nullptr;
      } else {
        currentTrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        trackInfo->tagBremCandidate(false);
      }
    }
    return;
  }

  // If the brem photon doesn't undergo any reaction in the target, stop
  // processing the rest of the event if the particle is exiting the
  // target region.
  if (secondaries->size() == 0) {
    /**
     * Check if the electron will be exiting the target
     *
     * The 'recoil_PV' volume name is automatically constructed by Geant4's
     * GDML parser and was found by inspecting the geometry using a
     * visualization. This Physical Volume (PV) is associated with the
     * recoil parent volume and so it will break if the recoil parent volume
     * changes its name.
     *
     * We also check for 'World_PV' because in later geometries, there is
     * an air gap between the target region and the recoil tracker.
     */
    if (auto volume{track->GetNextVolume()->GetName()};
        volume.compareTo("recoil_PV") == 0 or
        volume.compareTo("World_PV") == 0) {
      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        currentTrack_ = nullptr;
      } else {
        currentTrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        trackInfo->tagBremCandidate(false);
      }
    }
    return;
  } else {
    // If the brem gamma interacts and produced secondaries, get the
    // process used to create them.
    G4String processName =
        secondaries->at(0)->GetCreatorProcess()->GetProcessName();

    // Only record the process that is being biased
    if (!processName.contains(process_)) {
      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        currentTrack_ = nullptr;
      } else {
        currentTrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        trackInfo->tagBremCandidate(false);
      }
      return;
    }

    if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
      std::cout << "[ TargetProcessFilter ]: "
              << G4EventManager::GetEventManager()
                     ->GetConstCurrentEvent()
                     ->GetEventID()
              << " Brem photon produced " << secondaries->size()
              << " particle via " << processName << " process." << std::endl;
    }
    trackInfo->tagBremCandidate(false);
    trackInfo->setSaveFlag(true);
    trackInfo->tagPNGamma();
    getEventInfo()->decBremCandidateCount();
  }
}

void TargetProcessFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing, TargetProcessFilter)
