/**
 * @file TargetENProcessFilter.cxx
 * @brief Class defining a UserActionPlugin that biases Geant4 to only process
 *        events which involve an electronuclear reaction in the target
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetENProcessFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"

namespace biasing {

TargetENProcessFilter::TargetENProcessFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoilEnergyThreshold_ = parameters.getParameter<double>("recoilThreshold");
}

TargetENProcessFilter::~TargetENProcessFilter() {}

void TargetENProcessFilter::stepping(const G4Step* step) {
  if (reactionOccurred_) return;

  // Get the track associated with this step.
  G4Track* track = step->GetTrack();

  // Only process tracks whose parent is the primary particle
  if (track->GetParentID() != 0) return;

  // get the PDGID of the track.
  G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

  // Make sure that the particle being processed is an electron.
  if (pdgID != 11) return;  // Throw an exception

  // Get the volume the particle is in.
  G4VPhysicalVolume* volume = track->GetVolume();
  G4String volumeName = volume->GetName();

  // If the particle isn't in the target, don't continue with the processing.
  if (volumeName.compareTo(volumeName_) != 0) return;

  /*std::cout << "*******************************" << std::endl;
  std::cout << "*   Step " << track->GetCurrentStepNumber() << std::endl;
  std::cout << "********************************" << std::endl;*/

  if (track->GetMomentum().mag() > recoilEnergyThreshold_) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }

  // Get the particles daughters.
  const G4TrackVector* secondaries = step->GetSecondary();

  // If the brem photon doesn't undergo any reaction in the target, stop
  // processing the rest of the event.
  if (secondaries->size() == 0) {
    /*std::cout << "[ TargetENProcessFilter ]: "
                << "Electron did not interact in the target. --> Postponing
       tracks."
                << std::endl;*/

    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  } else {
    G4String processName =
        secondaries->at(0)->GetCreatorProcess()->GetProcessName();

    /*std::cout << "[ TargetENProcessFilter ]: "
              << "Electron produced " << secondaries->size()
              << " particle via " << processName << " process."
              << std::endl;*/

    // Only record the process that is being biased
    if (!processName.contains(process_)) {
      /*std::cout << "[ TargetENProcessFilter ]: "
                << "Process was not " << BiasingMessenger::getProcess() << "-->
         Killing all tracks!"
                << std::endl;*/

      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    std::cout << "[ TargetENProcessFilter ]: "
              << "Electronuclear reaction resulted in " << secondaries->size()
              << " particles via " << processName << " process." << std::endl;
    // BiasingMessenger::setEventWeight(track->GetWeight());
    reactionOccurred_ = true;
  }
}

void TargetENProcessFilter::EndOfEventAction(const G4Event*) {
  reactionOccurred_ = false;
}
}  // namespace biasing

DECLARE_ACTION(biasing, TargetENProcessFilter)
