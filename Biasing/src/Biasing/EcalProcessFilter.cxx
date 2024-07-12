
#include "Biasing/EcalProcessFilter.h"
/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4BiasingProcessInterface.hh"
#include "G4EventManager.hh"
#include "G4Gamma.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4ProcessTable.hh"
#include "G4RegionStore.hh"
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
  region_ = G4RegionStore::GetInstance()->GetRegion("CalorimeterRegion");

  hcal_pv_ = G4PhysicalVolumeStore::GetInstance()->GetVolume("hcal_PV");
  if (region_ == nullptr) {
    throw 32;
  }
  if (hcal_pv_ == nullptr) {
    throw 'c';
  }
  G4ProcessTable* processTable = G4ProcessTable::GetProcessTable();
  processPtr_ = processTable->FindProcess(process_, G4Gamma::Definition());
  if (processPtr_ == nullptr) {
    throw "cs";
  }
  G4cout << "Found process " << processPtr_->GetProcessName() << G4endl;
  G4cin.get();
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

  if (track->GetParticleDefinition() != G4Gamma::Definition()) {
    return;
  }

  // Get the track info and check if this track is a brem candidate
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  if ((trackInfo != nullptr) && !trackInfo->isBremCandidate()) return;

  // Get the particles daughters.
  auto secondaries{step->GetSecondary()};

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the calorimeter region.
  if (region_ != track->GetVolume()->GetLogicalVolume()->GetRegion()) {
    // if (auto region{
    //         track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
    //     region.compareTo("CalorimeterRegion") != 0) {
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
    /**
     * Check if the photon will be exiting the ecal
     *
     * The 'hcal_PV' volume name is automatically constructed by Geant4's
     * GDML parser and was found by inspecting the geometry using a
     * visualization. This Physical Volume (PV) is associated with the
     * hcal parent volume and so it will break if the hcal parent volume
     * changes its name.
     */
    // if (auto volume{track->GetNextVolume()->GetName()};
    //     volume.compareTo("hcal_PV") == 0) {
    if (track->GetNextVolume() == hcal_pv_) {
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
    // auto
    // processName{secondaries->at(0)->GetCreatorProcess()->GetProcessName()};
    const G4VProcess* process{secondaries->at(0)->GetCreatorProcess()};
    const G4VProcess* unwrapped_process{process};
    auto wrapped_process {dynamic_cast<const G4BiasingProcessInterface*>(process)};

    if (wrapped_process != nullptr) {
      unwrapped_process = wrapped_process->GetWrappedProcess();
    }
    // Only record the process that is being biased
    // if (!processName.contains(process_)) {
    if (unwrapped_process != processPtr_) {
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

  // ldmx_log(debug)
  //     << "[ EcalProcessFilter ]: "
  //     << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
  //     << " Brem photon produced " << secondaries->size() << " particle via "
  //     << process->GetProcessName() << " process.";
  trackInfo->tagBremCandidate(false);
  trackInfo->setSaveFlag(true);
  trackInfo->tagPNGamma();
  getEventInfo()->decBremCandidateCount();
}
}
}  // namespace biasing

DECLARE_ACTION(biasing, EcalProcessFilter)
