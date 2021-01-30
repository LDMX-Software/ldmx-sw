/*
 * @file DarkBremFilter.cxx
 * @class DarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out
 *        events that don't result in a dark brem within a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/DarkBremFilter.h"

#include "SimCore/G4APrime.h"

namespace biasing {

DarkBremFilter::DarkBremFilter(const std::string& name,
                               framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  volumeName_ = parameters.getParameter<std::string>("volume");
  verbosity_ = parameters.getParameter<int>("verbosity");

  // re-set verbosity and volumes to reasonable defaults
  if (verbosity_ < 0) verbosity_ = 0;
  if (volumeName_.empty()) volumeName_ = "target_PV";
}

DarkBremFilter::~DarkBremFilter() {}

G4ClassificationOfNewTrack DarkBremFilter::ClassifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  // get the PDGID of the track.
  G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

  // Get the particle type.
  G4String particleName = track->GetParticleDefinition()->GetParticleName();

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  if (track->GetTrackID() == 1 && pdgID == 11) {
    if (verbosity_ > 2) {
      std::cout << "[ DarkBremFilter ]: Pushing track to waiting stack."
                << std::endl;
    }
    return fWaiting;
  }

  return classification;
}

void DarkBremFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  G4Track* track = step->GetTrack();

  // Only process the primary electron track
  if (track->GetParentID() != 0) return;

  // get the PDGID of the track.
  G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

  // Make sure that the particle being processed is an electron.
  if (pdgID != 11) return;

  // Get the volume the particle is in.
  G4VPhysicalVolume* volume = track->GetVolume();
  G4String volumeName = volume->GetName();

  // If the particle isn't in the given volume, don't continue with the
  // processing.
  if (not volumeName.contains(volumeName_.c_str())) return;

  // Get the particle type.
  G4String particleName = track->GetParticleDefinition()->GetParticleName();

  if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
    // primary electron is exiting the volume.

    // Get the particles daughters.
    const G4TrackVector* secondaries = step->GetSecondary();

    if (secondaries->size() == 0) {
      // If the particle didn't produce any secondaries, stop processing the
      // event.
      if (verbosity_ > 1) {
        std::cout
            << "[ DarkBremFilter ]: "
            << "Primary did not produce secondaries --> Killing primary track!"
            << std::endl;
      }

      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    } else if (not hasAPrime(secondaries)) {
      // If the particle din't produce an A Prime, stop processing the event
      if (verbosity_ > 1) {
        std::cout << "[ DarkBremFilter ]: "
                  << "No dark brem in " << volumeName_ << " --> Aborting event."
                  << std::endl;
      }

      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

  } else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) {
    // primary electron stopped inside of volume
    const G4TrackVector* secondaries = step->GetSecondary();
    if (not hasAPrime(secondaries)) {
      // If the particle din't produce an A Prime, stop processing the event
      if (verbosity_ > 1) {
        std::cout << "[ DarkBremFilter ]: "
                  << "Electron never made it out of the " << volumeName_
                  << " --> Killing all tracks!" << std::endl;
      }

      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
  }  // check if particle is leaving volume or stopped within it

}  // stepping

bool DarkBremFilter::hasAPrime(const G4TrackVector* secondaries) const {
  for (auto& secondary_track : *secondaries) {
    if (secondary_track->GetParticleDefinition() == G4APrime::APrime())
      return true;
  }
  return false;
}
}  // namespace biasing

DECLARE_ACTION(biasing, DarkBremFilter)
