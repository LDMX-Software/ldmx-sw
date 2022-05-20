
#include "Biasing/TargetProcessFilter.h"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"

#include "g4fire/UserTrackInformation.h"

namespace biasing {

TargetProcessFilter::TargetProcessFilter(
    const std::string &name, fire::config::Parameters &parameters)
    : g4fire::UserAction(name, parameters) {
  process_ = parameters.get<std::string>("process");
}

G4ClassificationOfNewTrack TargetProcessFilter::ClassifyNewTrack(
    const G4Track *track, const G4ClassificationOfNewTrack &currentTrackClass) {
  if (track == ctrack_) {
    ctrack_ = nullptr;
    return fWaiting;
  }

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  return classification;
}

void TargetProcessFilter::stepping(const G4Step *step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Get the track info and check if this track is a brem candidate
  auto track_info{g4fire::UserTrackInformation::get(track)};
  if ((track_info != nullptr) && !track_info->isBremCandidate())
    return;

  // Continue processing the particle only if it's in the target region.
  if (auto region{
          track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("target") != 0)
    return;

  // Get the particles daughters.
  auto secondaries{step->GetSecondary()};

  // If the brem photon doesn't undergo any reaction in the target, stop
  // processing the rest of the event if the particle is exiting the
  // target region.
  if (secondaries->size() == 0) {
    if (auto nregion{
            track->GetNextVolume()->GetLogicalVolume()->GetRegion()->GetName()};
        nregion.compareTo("target") != 0) {


      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        ctrack_ = nullptr;
      } else {
        ctrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
      }
      return;
    }
  } else {
    G4String processName =
        secondaries->at(0)->GetCreatorProcess()->GetProcessName();

    // Only record the process that is being biased
    if (!processName.contains(process_)) {
      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        ctrack_ = nullptr;
      } else {
        ctrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
      }
      return;
    }

    std::cout << "[ TargetProcessFilter ]: "
              << "Brem photon produced " << secondaries->size()
              << " particle via " << processName << " process." << std::endl;
    track_info->tagBremCandidate(false);
    track_info->tagPNGamma();
    getEventInfo()->decBremCandidateCount();
  }
}

void TargetProcessFilter::EndOfEventAction(const G4Event *) {}
} // namespace biasing

DECLARE_ACTION(biasing, TargetProcessFilter)
