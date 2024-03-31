
#include "Biasing/Utility/StepPrinter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Step.hh"

namespace biasing {
namespace utility {

StepPrinter::StepPrinter(const std::string& name,
                         framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  trackID_ = parameters.getParameter<int>("track_id");
  processName_ = parameters.getParameter<std::string>("process_name");
  depth_ = parameters.getParameter<int>("depth");
}

bool StepPrinter::isDescendent(const G4Track* track) const {
  auto trackID{track->GetTrackID()};
  int currentDepth{0};
  int currentTrackID{trackID};
  // Walk the tree until we either no longer have a parent or we reach the
  // desired depth
  while (currentDepth < depth_ &&
         trackParents_.find(currentTrackID) != trackParents_.end()) {
    // See if we have encountered the parent of the current track
    //
    // operator[] is not const, so we need to use at()
    currentTrackID = trackParents_.at(currentTrackID);
    if (currentTrackID == trackID_) {
      // If one of the parents is the track of interest, we are done!
      return true;
    }
    currentTrackID = track->GetParentID();
    currentDepth++;
  }
  return false;
}
void StepPrinter::stepping(const G4Step* step) {
  // Get the track associated with this step
  auto track{step->GetTrack()};

  const auto trackID{track->GetTrackID()};
  const auto parent{track->GetParentID()};
  // Don't bother filling the map if we aren't going to use it
  if (depth_ > 0) {
    trackParents_[trackID] = parent;
  }

  auto process{track->GetCreatorProcess()};
  std::string processName{process ? process->GetProcessName() : "Primary"};
  // Unwrap biasing part of process name if present
  if (processName.find("biasWrapper") != std::string::npos) {
    std::size_t pos = processName.find_first_of("(") + 1;
    processName = processName.substr(pos, processName.size() - pos - 1);
  }

  // This could be a negated condition, but it is easier to read this way
  //
  if (trackID == trackID_ || // We are the track of interest
      isDescendent(track) || // We are a descendent of the track of interest
      processName == processName_ // The parent process was the process of interest
  ) {
    // This is an interesting track -> Carry on processing
  }  else {
    return;
  }
  // Get the particle name.
  const auto particleName{track->GetParticleDefinition()->GetParticleName()};

  // Get the energy of the particle
  const auto energy{step->GetPostStepPoint()->GetTotalEnergy()};

  // Get the volume the particle is in.
  auto volume{track->GetVolume()};
  auto volumeName{volume->GetName()};

  // Get the next volume (can fail if current volume is WorldPV and next is
  // outside the world)
  auto nextVolume{track->GetNextVolume() ? track->GetNextVolume()->GetName()
                                         : "undefined"};

  // Get the region
  auto regionName{volume->GetLogicalVolume()->GetRegion()->GetName()};

  std::cout << " Step " << track->GetCurrentStepNumber() << " ("
            << track->GetParticleDefinition()->GetParticleName() << ") {"
            << " Energy: " << energy << " Track ID: " << track->GetTrackID()
            << " Particle currently in: " << volumeName
            << " Region: " << regionName << " Next volume: " << nextVolume
            << " Weight: " << track->GetWeight() << " Parent: " << parent
            << " (" << processName << ") "
            << " Children:";
  for (auto const& child : *(step->GetSecondaryInCurrentStep())) {
    std::cout << " (" << child->GetTotalEnergy()
              << "): " << child->GetParticleDefinition()->GetPDGEncoding();
  }

  std::cout << " }" << std::endl;
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, StepPrinter)
