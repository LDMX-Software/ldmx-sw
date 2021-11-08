
#include "Biasing/EcalProcessFilter.h"

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

EcalProcessFilter::EcalProcessFilter(const std::string& name,
                                     framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  process_ = parameters.getParameter<std::string>("process");
}

EcalProcessFilter::~EcalProcessFilter() {}

G4ClassificationOfNewTrack EcalProcessFilter::ClassifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  // Get the particle type.
  G4String particleName = track->GetParticleDefinition()->GetParticleName();

  if (track == currentTrack_) {
    /*
    std::cout << "[ EcalProcessFilter ]: "
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

void EcalProcessFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted())
    return;

  // Get the track info and check if this track is a brem candidate
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  if ((trackInfo != nullptr) && !trackInfo->isBremCandidate()) return;

  // Get the particles daughters.
  auto secondaries{step->GetSecondary()};

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the calorimeter region.
  if (auto region{
          track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("CalorimeterRegion") != 0) {
    // If secondaries were produced outside of the volume of interest,
    // and there aren't additional brems to process, abort the
    // event.  Otherwise, suspend the track and move on to the next
    // brem.
    if (secondaries->size() != 0) {
      /*
      std::cout << "[ EcalProcessFilter ]: "
            <<
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
            << " secondaries outside ecal...";
      */
      if (getEventInfo()->bremCandidateCount() == 1) {
        // std::cout << "aborting the event." << std::endl;
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        currentTrack_ = nullptr;
      } else {
        /*
        std::cout << "suspending the track " << track->GetTrackID()
            << " , " << getEventInfo()->bremCandidateCount() << " brems left."
            << std::endl;
        */
        currentTrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        trackInfo->tagBremCandidate(false);
      }
    }
    return;
  }

  // If the particle doesn't interact, then move on to the next step.
  if (secondaries->size() == 0) {
    // Check if the photon will be exiting the ecal
    if (auto volume{track->GetNextVolume()->GetName()};
        volume.compareTo("hcal_PV") == 0) {
      /*
      std::cout << "[ EcalProcessFilter ]: "
            <<
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
            << " no secondaries when leaving ecal...";
      */
      if (getEventInfo()->bremCandidateCount() == 1) {
        // std::cout << "aborting the event." << std::endl;
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        currentTrack_ = nullptr;
      } else {
        /*
        std::cout << "suspending the track " << track->GetTrackID()
            << " , " << getEventInfo()->bremCandidateCount() << " brems left."
            << std::endl;
        */
        currentTrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        trackInfo->tagBremCandidate(false);
      }
    }

    return;
  } else {
    // If the brem gamma interacts and produces secondaries, get the
    // process used to create them.
    auto processName{secondaries->at(0)->GetCreatorProcess()->GetProcessName()};

    // Only record the process that is being biased
    if (!processName.contains(process_)) {
      /*
      std::cout << "[ EcalProcessFilter ]: "
            <<
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
            << " not PN products...";
      */
      if (getEventInfo()->bremCandidateCount() == 1) {
        // std::cout << "aborting the event." << std::endl;
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        currentTrack_ = nullptr;
      } else {
        /*
        std::cout << "suspending the track " << track->GetTrackID()
            << " , " << getEventInfo()->bremCandidateCount() << " brems left."
            << std::endl;
        */
        currentTrack_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        trackInfo->tagBremCandidate(false);
      }
      return;
    }

    std::cout << "[ EcalProcessFilter ]: "
              << G4EventManager::GetEventManager()
                     ->GetConstCurrentEvent()
                     ->GetEventID()
              << " Brem photon produced " << secondaries->size()
              << " particle via " << processName << " process." << std::endl;
    trackInfo->tagBremCandidate(false);
    trackInfo->setSaveFlag(true);
    trackInfo->tagPNGamma();
    getEventInfo()->decBremCandidateCount();
  }
}
}  // namespace biasing

DECLARE_ACTION(biasing, EcalProcessFilter)
