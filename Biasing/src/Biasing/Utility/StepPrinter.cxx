
#include "Biasing/Utility/StepPrinter.h"

#include "G4Step.hh"

namespace biasing {
namespace utility {

StepPrinter::StepPrinter(const std::string& name, fire::config::Parameters& parameters)
    : g4fire::UserAction(name, parameters) {
  track_id_ = parameters.get<int>("track_id");
}

StepPrinter::~StepPrinter() {}

void StepPrinter::stepping(const G4Step* step) {
  // Get the track associated with this step
  auto track{step->GetTrack()};

  if (auto track_id{track->GetTrackID()};
      (track_id_ > 0) && (track_id != track_id_))
    return;

  // Get the particle name.
  auto particleName{track->GetParticleDefinition()->GetParticleName()};

  // Get the energy of the particle
  auto energy{step->GetPostStepPoint()->GetTotalEnergy()};

  // Get the volume the particle is in.
  auto volume{track->GetVolume()->GetName()};

  // Get the next volume
  auto nextVolume{track->GetNextVolume()->GetName()};

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
