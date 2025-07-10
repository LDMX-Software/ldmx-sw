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
#include "SimCore/G4User/UserEventInformation.h"
#include "SimCore/G4User/UserTrackInformation.h"
#include "SimCore/G4User/VolumeChecks.h"

namespace biasing {

DeepEcalProcessFilter::DeepEcalProcessFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  bias_threshold_ = parameters.getParameter<double>("bias_threshold");
  processes_ = parameters.getParameter<std::vector<std::string>>("processes");
  ecal_min_Z_ = parameters.getParameter<double>("ecal_min_Z");
  require_photon_fromTarget_ =
      parameters.getParameter<bool>("require_photon_fromTarget");
}

void DeepEcalProcessFilter::BeginOfEventAction(const G4Event* event) {
  hasDeepEcalProcess_ = false;
  photonFromTarget_ = false;
}

void DeepEcalProcessFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Check the creation process and PDG ID of the particle
  auto processName = track->GetCreatorProcess()
                         ? track->GetCreatorProcess()->GetProcessName()
                         : "unknown";
  auto PDGid = track->GetParticleDefinition()->GetPDGEncoding();

  // Skip the steps that are for the recoil electron
  // PrimaryToEcalFilter made sure there is a fiducial e-
  if (processName.contains("unknown")) return;

  // Energy of the particle is below threshold, move to next step
  if (track->GetKineticEnergy() < bias_threshold_) {
    return;
  }

  // Check in which volume the particle is currently
  auto phys_vol{track->GetVolume()};
  auto volume{phys_vol ? phys_vol->GetLogicalVolume() : nullptr};
  auto volume_name{volume ? volume->GetName() : "undefined"};

  auto trackInfo{simcore::UserTrackInformation::get(track)};
  // Tag the brem photon from the primary electron
  if (processName.contains("eBrem") and (track->GetParentID() == 1)) {
    trackInfo->tagBremCandidate();
    getEventInfo()->incBremCandidateCount();
    trackInfo->setSaveFlag(true);
    if (volume_name.contains("target")) {
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
    // ldmx_log(debug) << "Allowed processed " << process << " now we have " <<
    // processName;
    if (processName.contains(process)) {
      hasProcessNeeded = true;
      break;
    }
  }
  // skip this step if it does not have any of the processes needed
  if (not hasProcessNeeded) return;

  auto is_in_ecal =
      simcore::g4user::volumechecks::isInEcal(volume, volume_name);

  // Skip this step if it does not have the processes needed
  // or if it's not in the ECAL
  if (not is_in_ecal) return;

  // Check the z position of the particle, and
  // flag if it is deeper than the min Z we are considering (but in ECAL)
  auto zPosition = step->GetPreStepPoint()->GetPosition().z();
  // Printout for testing
  if (zPosition > (0.75 * ecal_min_Z_)) {
    ldmx_log(debug) << " Particle ID " << PDGid << " with energy "
                    << track->GetKineticEnergy() << " on " << volume << " from "
                    << processName << " at Z = " << zPosition;
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

DECLARE_ACTION(biasing::DeepEcalProcessFilter)
