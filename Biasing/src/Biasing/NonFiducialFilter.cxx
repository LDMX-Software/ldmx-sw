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
#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackInformation.h"

namespace biasing {

bool nonFiducial_ = false;

NonFiducialFilter::NonFiducialFilter(const std::string& name,
                                     framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoil_max_p_ = parameters.getParameter<double>("recoil_max_p");
  abort_fiducial_ = parameters.getParameter<bool>("abort_fiducial");
}

void NonFiducialFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Get the PDG ID of the track and make sure it's an electron.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()};
      pdgID != 11) {
    return;
  }

  // Only process the primary electron track
  int parentID{step->GetTrack()->GetParentID()};
  if (parentID != 0) {
    return;
  }

  // Check in which volume the electron is currently
  auto volume{track->GetVolume()->GetLogicalVolume()
                  ? track->GetVolume()->GetLogicalVolume()->GetName()
                  : "undefined"};

  // Check if the track is tagged.
  auto electronCheck{simcore::UserTrackInformation::get(track)};
  if (electronCheck->isRecoilElectron() == true) {
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
    auto isInEcal{((volume.contains("Si") || volume.contains("W") ||
                    volume.contains("PCB") || volume.contains("strongback") ||
                    volume.contains("Glue") || volume.contains("CFMix") ||
                    volume.contains("Al") || volume.contains("C")) &&
                   volume.contains("volume")) ||
                  (volume.contains("nohole_motherboard"))};

    // isInEcal should be taken from
    // simcore::logical_volume_tests::isInEcal(volume) but for now it's under
    // its own namespace so I cannot reach it here see issue
    // https://github.com/LDMX-Software/ldmx-sw/issues/1286
    if (abort_fiducial_ && isInEcal) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      ldmx_log(debug) << ">> This event is fiducial, exiting";
      nonFiducial_ = false;
      return;
    }
    // I comment the following debug out since it would print per step and it's
    // hard to read but it could be otherwise useful if somebody wants to do a
    // step-by-step debugging ldmx_log(debug) << "  >> In this step this is
    // non-fiducial, keeping it so far";
    nonFiducial_ = true;
    return;
  } else {
    // Check if the particle enters the recoil tracker.
    if (volume.compareTo("recoil") == 0) {
      /* Tag the tracks that:
       1) Have a recoil electron
       2) Enter/Exit the Target */
      auto trackInfo{simcore::UserTrackInformation::get(track)};
      trackInfo->tagRecoilElectron();  // tag the target recoil electron
      ldmx_log(debug) << "  >> This track is the recoil electron, tagging it";
      return;
    }
  }
}

void NonFiducialFilter::EndOfEventAction(const G4Event*) {
  if (nonFiducial_) {
    ldmx_log(debug) << "  >> This event is non-fiducial in ECAL, keeping it";
  } else {
    ldmx_log(debug) << ">> This event is fiducial, exiting";
  }
}
}  // namespace biasing

DECLARE_ACTION(biasing, NonFiducialFilter)
