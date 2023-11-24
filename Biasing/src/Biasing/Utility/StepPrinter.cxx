
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

  if (auto trackID{track->GetTrackID()};
      (trackID_ > 0) && (trackID != trackID_))
    return;

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
  auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};

  std::cout << " Step " << track->GetCurrentStepNumber() << " {"
            << " Energy: " << energy << " Track ID: " << track->GetTrackID()
            << " Particle currently in: " << volume << " Region: " << region
            << " Next volume: " << nextVolume
            << " Weight: " << track->GetWeight() << " Children:";
  for (auto const& track : *(step->GetSecondaryInCurrentStep()))
    std::cout << " " << track->GetParticleDefinition()->GetPDGEncoding();

  std::cout << " }" << std::endl;
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, StepPrinter)
