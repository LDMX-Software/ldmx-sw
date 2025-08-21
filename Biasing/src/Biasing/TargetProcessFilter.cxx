/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
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
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/PtrRetrieval.h"
#include "SimCore/G4User/UserTrackInformation.h"

namespace biasing {

TargetProcessFilter::TargetProcessFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  process_ = parameters.get<std::string>("process");
}

G4ClassificationOfNewTrack TargetProcessFilter::classifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  if (track == current_track_) {
    current_track_ = nullptr;
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
  auto track_info{simcore::UserTrackInformation::get(track)};
  if ((track_info != nullptr) && !track_info->isBremCandidate()) return;

  // Get the particles daughters.
  auto secondaries{step->GetSecondary()};

  // Get the region the particle is currently in. Continue processing
  // the particle only if it's in the target region.
  static auto target_region =
      simcore::g4user::ptrretrieval::getRegion("target");
  if (!target_region) {
    ldmx_log(warn) << "Region 'target' not found in Geant4 region store";
  }
  auto phy_vol{track->GetVolume()};
  auto log_vol{phy_vol ? phy_vol->GetLogicalVolume() : nullptr};
  auto current_region{log_vol ? log_vol->GetRegion() : nullptr};
  if (current_region != target_region) {
    // If secondaries were produced outside of the volume of interest,
    // and there aren't additional brems to process, abort the event.
    // Otherwise, suspend the track and move on to the next brem.
    if (secondaries->size() != 0) {
      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        current_track_ = nullptr;
      } else {
        current_track_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
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
    static auto recoil_physical_volume =
        simcore::g4user::ptrretrieval::getPhysicalVolume("recoil_PV");
    static auto world_physical_volume =
        simcore::g4user::ptrretrieval::getPhysicalVolume("World_PV");

    if (!recoil_physical_volume) {
      ldmx_log(warn) << "Volume 'recoil_PV' not found in Geant4 volume store";
    }
    if (!world_physical_volume) {
      ldmx_log(warn) << "Volume 'World_PV' not found in Geant4 volume store";
    }

    auto current_volume = track->GetNextVolume();
    if (current_volume == recoil_physical_volume or
        current_volume == world_physical_volume) {
      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        current_track_ = nullptr;
      } else {
        current_track_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
      }
    }
    return;
  } else {
    // If the brem gamma interacts and produced secondaries, get the
    // process used to create them.
    G4String process_name =
        secondaries->at(0)->GetCreatorProcess()->GetProcessName();

    // Only record the process that is being biased
    if (!process_name.contains(process_)) {
      if (getEventInfo()->bremCandidateCount() == 1) {
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        current_track_ = nullptr;
      } else {
        current_track_ = track;
        track->SetTrackStatus(fSuspend);
        getEventInfo()->decBremCandidateCount();
        track_info->tagBremCandidate(false);
      }
      return;
    }

    if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
      std::cout << "[ TargetProcessFilter ]: "
                << G4EventManager::GetEventManager()
                       ->GetConstCurrentEvent()
                       ->GetEventID()
                << " Brem photon produced " << secondaries->size()
                << " particle via " << process_name << " process." << std::endl;
    }
    track_info->tagBremCandidate(false);
    track_info->setSaveFlag(true);
    track_info->tagPNGamma();
    getEventInfo()->decBremCandidateCount();
  }
}

void TargetProcessFilter::endOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing::TargetProcessFilter)
