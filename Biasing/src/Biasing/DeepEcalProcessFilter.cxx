#include "Biasing/DeepEcalProcessFilter.h"

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


DeepEcalProcessFilter::DeepEcalProcessFilter(const std::string& name,
                                     framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  bias_threshold_ = parameters.getParameter<double>("bias_threshold");
  processes_ = parameters.getParameter<std::vector<std::string>>("processes");
  ecal_min_Z_ = parameters.getParameter<double>("ecal_min_Z");
  require_photon_fromTarget_ = parameters.getParameter<bool>("require_photon_fromTarget");
}


void DeepEcalProcessFilter::BeginOfEventAction(const G4Event* event) {
  hasDeepEcalProcess_ = false;
  photonFromTarget_ = false;
}

void DeepEcalProcessFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};
  
  // Check the creation process and PDG ID of the particle
  auto processName = track->GetCreatorProcess() ? track->GetCreatorProcess()->GetProcessName() : "unknown";
  auto PDGid = track->GetParticleDefinition()->GetPDGEncoding();
  
  // Skip the steps that are for the recoil electron
  // PrimaryToEcalFilter made sure there is a fiducial e-
  if (processName.contains("unknown")) return;
  
  // Energy of the particle is below threshold, move to next step
  if (track->GetKineticEnergy() < bias_threshold_) {
    return;
  }
  
  // Check in which volume the particle is currently
  auto volume{track->GetVolume()->GetLogicalVolume()
    ? track->GetVolume()->GetLogicalVolume()->GetName()
    : "undefined"};
  
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  // Tag the brem photon from the primary electron
  if (processName.contains("eBrem") and (track->GetParentID() == 1)) {
    trackInfo->tagBremCandidate();
    getEventInfo()->incBremCandidateCount();
    trackInfo->setSaveFlag(true);
    if (volume.contains("target")) {
      photonFromTarget_ = true;
    }
  }
  
  // If we require that the photon comes from the target and
  // and if it does not, let's skip the event
  if (require_photon_fromTarget_ and !photonFromTarget_) {
    return;
  }

  // Tag if the event has the processes we are looking for
  bool hasProcessNeeded{false};
  for (auto& process : processes_) {
    // ldmx_log(debug) << "Allowed processed " << process << " now we have " << processName;
    if (processName.contains(process)) {
      hasProcessNeeded = true;
      break;
    }
  }
  // skip this step if it does not have any of the processes needed
  if (not hasProcessNeeded) return;

  // isInEcal should be taken from
  // simcore::logical_volume_tests::isInEcal(volume) but for now it's under
  // its own namespace so I cannot reach it here, see issue
  // https://github.com/LDMX-Software/ldmx-sw/issues/1286
  auto isInEcal{((volume.contains("Si") || volume.contains("W") ||
                  volume.contains("PCB") || volume.contains("strongback") ||
                  volume.contains("Glue") || volume.contains("CFMix") ||
                  volume.contains("Al") || volume.contains("C")) &&
                 volume.contains("volume")) ||
    (volume.contains("nohole_motherboard"))};
  
  // Skip this step if it does not have the processes needed
  // or if it's not in the ECAL
  if (not isInEcal) return;
  
  // Check the z position of the particle, and
  // flag if it is deeper than the min Z we are considering (but in ECAL)
  auto zPosition = step->GetPreStepPoint()->GetPosition().z();
  // Printout for testing
  if (zPosition > (0.75*ecal_min_Z_)) {
    ldmx_log(debug) << " Particle ID " << PDGid << " with energy " << track->GetKineticEnergy() << " on " << volume << " from " << processName << " at Z = " << zPosition;
    if (zPosition > ecal_min_Z_) {
      hasDeepEcalProcess_ = true;
    }
  }
  return;
}

void DeepEcalProcessFilter::NewStage() {
  if (hasDeepEcalProcess_) {
    ldmx_log(debug) << "> Event with a hard deep conversion found, yaaay!";
    ldmx_log(debug) << "> -----------------------------------------";
  } else {
//    ldmx_log(debug) << "> -----------------------------------------";
    G4RunManager::GetRunManager()->AbortEvent();
  }
}
}  // namespace biasing

DECLARE_ACTION(biasing, DeepEcalProcessFilter)
