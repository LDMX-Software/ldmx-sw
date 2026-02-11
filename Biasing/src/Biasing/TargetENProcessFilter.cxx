/**
 * @file TargetENProcessFilter.cxx
 * @brief Class defining a UserActionPlugin that biases Geant4 to only process
 *        events which involve an electronuclear reaction in the target
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */
/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/TargetENProcessFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/PtrRetrieval.h"

namespace biasing {

TargetENProcessFilter::TargetENProcessFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoil_energy_threshold_ = parameters.get<double>("recoil_threshold");
}

TargetENProcessFilter::~TargetENProcessFilter() {}

void TargetENProcessFilter::stepping(const G4Step* step) {
  if (reaction_occurred_) return;

  // Get the track associated with this step.
  G4Track* track = step->GetTrack();

  // Only process tracks whose parent is the primary particle
  if (track->GetParentID() != 0) return;

  // get the PDGID of the track.
  G4int pdg_id = track->GetParticleDefinition()->GetPDGEncoding();

  // Make sure that the particle being processed is an electron.
  if (pdg_id != 11) return;  // Throw an exception

  // Get the volume the particle is in.
  G4VPhysicalVolume* track_volume = track->GetVolume();
  auto target_volume =
      simcore::g4user::ptrretrieval::getPhysicalVolume("target_PV");
  if (!target_volume) {
    ldmx_log(warn) << "Volume 'target_PV' not found in Geant4 volume store";
  }
  // If the particle isn't in the target, don't continue with the processing.
  if (track_volume != target_volume) return;

  // ldmx_log(trace)<< "*   Step " << track->GetCurrentStepNumber();

  if (track->GetMomentum().mag() > recoil_energy_threshold_) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }

  // Get the particles daughters.
  const G4TrackVector* secondaries = step->GetSecondary();

  // If the brem photon doesn't undergo any reaction in the target, stop
  // processing the rest of the event.
  if (secondaries->size() == 0) {
    ldmx_log(debug)
        << "Electron did not interact in the target. --> Postponing tracks.";

    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  } else {
    G4String process_name =
        secondaries->at(0)->GetCreatorProcess()->GetProcessName();

    ldmx_log(debug) << "Electron produced " << secondaries->size()
                    << " particle via " << process_name << " process.";

    // Only record the process that is being biased
    if (!process_name.contains(process_)) {
      ldmx_log(debug) << "Process was not " << process_
                      << "--> Killing all tracks!";

      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    ldmx_log(info) << "Electronuclear reaction resulted in "
                   << secondaries->size() << " particles via " << process_name
                   << " process.";
    // BiasingMessenger::setEventWeight(track->GetWeight());
    reaction_occurred_ = true;
  }
}

void TargetENProcessFilter::EndOfEventAction(const G4Event*) {
  reaction_occurred_ = false;
}
}  // namespace biasing

DECLARE_ACTION(biasing::TargetENProcessFilter)
