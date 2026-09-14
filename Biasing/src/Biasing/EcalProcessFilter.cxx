/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
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
#include "SimCore/G4User/PtrRetrieval.h"
#include "SimCore/G4User/UserTrackInformation.h"

namespace biasing {

EcalProcessFilter::EcalProcessFilter(const std::string& name,
                                     framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  process_ = parameters.get<std::string>("process");
}

G4ClassificationOfNewTrack EcalProcessFilter::classifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  // Get the particle type.
  G4String particle_name = track->GetParticleDefinition()->GetParticleName();

  if (track == current_track_) {
    /*
    std::cout << "[ EcalProcessFilter ]: "
        << "Putting track " << track->GetTrackID()
        << " onto waiting stack." << std::endl;
    */
    current_track_ = nullptr;
    return fWaiting;
  }

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  return classification;
}

void EcalProcessFilter::stepping(const G4Step* step) {
  static auto calorimeter_region =
      simcore::g4user::ptrretrieval::getRegion("CalorimeterRegion");
  if (!calorimeter_region) {
    ldmx_log(warn)
        << "Region 'CalorimeterRegion' not found in Geant4 region store";
  }
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted())
    return;

  // Get the track info and check if this track is a brem candidate
  auto track_info{simcore::UserTrackInformation::get(track)};
  if ((track_info != nullptr) && !track_info->isBremCandidate()) return;

  // Get the particles daughters.
  auto secondaries{step->GetSecondary()};

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the calorimeter region.
  auto phys_vol{track->GetVolume()};
  auto lv{phys_vol ? phys_vol->GetLogicalVolume() : nullptr};
  auto region{lv ? lv->GetRegion() : nullptr};
  if (region != calorimeter_region) {
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
        current_track_ = nullptr;
      } else {
        /*
        std::cout << "suspending the track " << track->GetTrackID()
            << " , " << getEventInfo()->bremCandidateCount() << " brems left."
            << std::endl;
        */
        current_track_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
      }
    }
    return;
  }

  // If the particle doesn't interact, then move on to the next step.
  if (secondaries->size() == 0) {
    /**
     * Check if the photon will be exiting the ecal
     *
     * The 'hadronic_calorimeter' logical volume name is the parent
     * volume to all of the HCal components.
     */
    static auto volume_after_exiting_ecal =
        simcore::g4user::ptrretrieval::getLogicalVolume("hadronic_calorimeter");
    if (!volume_after_exiting_ecal) {
      ldmx_log(warn) << "Unable to find 'hadronic_calorimeter' logical volume.";
    }
    auto next_phys_vol = track->GetNextVolume();
    auto next_log_vol{next_phys_vol ? next_phys_vol->GetLogicalVolume()
                                    : nullptr};
    if (next_log_vol == volume_after_exiting_ecal) {
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
        current_track_ = nullptr;
      } else {
        /*
        std::cout << "suspending the track " << track->GetTrackID()
            << " , " << getEventInfo()->bremCandidateCount() << " brems left."
            << std::endl;
        */
        current_track_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
      }
    }

    return;
  } else {
    // If the brem gamma interacts and produces secondaries, get the
    // process used to create them.
    auto process_name{
        secondaries->at(0)->GetCreatorProcess()->GetProcessName()};

    // Only record the process that is being biased
    if (!process_name.contains(process_)) {
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
        current_track_ = nullptr;
      } else {
        /*
        std::cout << "suspending the track " << track->GetTrackID()
            << " , " << getEventInfo()->bremCandidateCount() << " brems left."
            << std::endl;
        */
        current_track_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
      }
      return;
    }

    ldmx_log(info) << " Brem photon produced " << secondaries->size()
                   << " particles via " << process_name << " process.";
    track_info->tagBremCandidate(false);
    track_info->setSaveFlag(true);
    track_info->tagPNGamma();
    getEventInfo()->decBremCandidateCount();
  }
}
}  // namespace biasing

DECLARE_ACTION(biasing::EcalProcessFilter)
