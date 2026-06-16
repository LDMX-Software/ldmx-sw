/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/NonFiducialFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4String.hh"
#include "G4Track.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/PtrRetrieval.h"
#include "SimCore/G4User/UserEventInformation.h"
#include "SimCore/G4User/UserTrackInformation.h"
#include "SimCore/G4User/VolumeChecks.h"

namespace biasing {

bool non_fiducial = false;

NonFiducialFilter::NonFiducialFilter(const std::string& name,
                                     framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoil_max_p_ = parameters.get<double>("recoil_max_p");
  abort_fiducial_ = parameters.get<bool>("abort_fiducial");
}

void NonFiducialFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Get the PDG ID of the track and make sure it's an electron.
  if (auto pdg_id{track->GetParticleDefinition()->GetPDGEncoding()};
      pdg_id != 11) {
    return;
  }

  // Only process the primary electron track
  int parent_id{step->GetTrack()->GetParentID()};
  if (parent_id != 0) {
    return;
  }

  // Check in which volume the electron is currently
  auto phys_vol = track->GetVolume();
  auto volume{phys_vol ? phys_vol->GetLogicalVolume() : nullptr};

  // Check if the track is tagged.
  auto electron_check{simcore::UserTrackInformation::get(track)};
  if (electron_check->isRecoilElectron() == true) {
    if (track->GetMomentum().mag() > recoil_max_p_) {
      // Kill the track if its momemntum is too high
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      ldmx_log(debug) << " Recoil track momentum is too high, expected to be "
                         "fiducial, exiting\n";
      return;
    }
    // Check if the track ever enters the ECal. If it does, kill the track and
    // abort the event.
    auto volume_name{volume ? volume->GetName() : "undefined"};
    auto is_in_ecal =
        simcore::g4user::volumechecks::isInEcal(volume, volume_name);
    if (abort_fiducial_ && is_in_ecal) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      ldmx_log(debug) << ">> This event is fiducial, exiting";
      non_fiducial = false;
      return;
    }
    // I comment the following debug out since it would print per step and it's
    // hard to read but it could be otherwise useful if somebody wants to do a
    // step-by-step debugging ldmx_log(debug) << "  >> In this step this is
    // non-fiducial, keeping it so far";
    non_fiducial = true;
    return;
  } else {
    // Check if the particle enters the recoil tracker.
    static auto recoil_volume =
        simcore::g4user::ptrretrieval::getLogicalVolume("recoil");
    if (!recoil_volume) {
      ldmx_log(warn) << "Volume 'recoil' not found in Geant4 volume store";
    }
    if (volume == recoil_volume) {
      /* Tag the tracks that:
       1) Have a recoil electron
       2) Enter/Exit the Target */
      auto track_info{simcore::UserTrackInformation::get(track)};
      track_info->tagRecoilElectron();  // tag the target recoil electron
      ldmx_log(debug) << "  >> This track is the recoil electron, tagging it";
      return;
    }
  }
}

void NonFiducialFilter::endOfEventAction(const G4Event*) {
  if (non_fiducial) {
    ldmx_log(debug) << "  >> This event is non-fiducial in ECAL, keeping it";
  } else {
    ldmx_log(debug) << ">> This event is fiducial, exiting";
  }
}
}  // namespace biasing

DECLARE_ACTION(biasing::NonFiducialFilter)
