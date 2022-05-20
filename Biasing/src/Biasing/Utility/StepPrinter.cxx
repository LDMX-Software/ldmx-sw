
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
  auto particle_name{track->GetParticleDefinition()->GetParticleName()};

  // Get the energy of the particle
  auto energy{step->GetPostStepPoint()->GetTotalEnergy()};

  // Get the volume the particle is in.
  auto volume{track->GetVolume()->GetName()};

  // Get the next volume
  auto next_volume{"None"}; 
  if (track->GetNextVolume() != nullptr) next_volume = track->GetNextVolume()->GetName(); 

  // Get the region
  auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
  
  std::cout << " Step " << track->GetCurrentStepNumber() << " {\n"
            << "\tEnergy: " << energy << "\n\tTrack ID: " << track->GetTrackID()
            << "\n\tParticle currently in: " << volume << "\n\tRegion: " << region
            << "\n\tNext volume: " << next_volume
            << "\n\tWeight: " << track->GetWeight() << "\n\tSecondaries:";
  for (auto const& sec : *(step->GetSecondary()))
    std::cout << " " << sec->GetParticleDefinition()->GetPDGEncoding();
  std::cout << "\n}" << std::endl; 
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, StepPrinter)
