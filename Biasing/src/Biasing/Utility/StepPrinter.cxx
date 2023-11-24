
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
}


void StepPrinter::stepping(const G4Step* step) {
  // Get the track associated with this step
  auto track{step->GetTrack()};

  const auto trackID{track->GetTrackID()};
  const auto parent{track->GetParentID()};
  auto process{track->GetCreatorProcess()};
  std::string processName{process ? process->GetProcessName() : "Primary"};
  // Unwrap biasing part of process name if present
  if (processName.find("biasWrapper") != std::string::npos) {
    std::size_t pos = processName.find_first_of("(") + 1;
    processName = processName.substr(pos, processName.size() - pos - 1);
  }

  // Print if any of the following conditions are true
  if (!((trackID == trackID_)  // We are the track of interest
        || (processName ==
            processName_)  // We were created by the process of interest
        || (depth > 0 &&
            parent == trackID_)  // Our parent was the track of interest
        )) {
    // We aren't an interesting track!
    return;
  }

  // Get the particle name.
  auto particleName{track->GetParticleDefinition()->GetParticleName()};

  // Get the energy of the particle
  auto energy{step->GetPostStepPoint()->GetTotalEnergy()};

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
            << track->GetParticleDefinition()->GetParticleName()
            << ") {"
            << " Energy: " << energy << " Track ID: " << track->GetTrackID()
            << " Particle currently in: " << volumeName << " Region: " << regionName
            << " Next volume: " << nextVolume
            << " Weight: " << track->GetWeight() << " Parent: " << parent << " (" << processName << ") "
            << " Children:";
  for (auto const& child : *(step->GetSecondaryInCurrentStep())) {
    std::cout << " (" << child->GetTotalEnergy() << "): "<< child->GetParticleDefinition()->GetPDGEncoding();
  }

  std::cout << " }" << std::endl;
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, StepPrinter)
