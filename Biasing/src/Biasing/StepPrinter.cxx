
#include "Biasing/StepPrinter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Step.hh"

namespace biasing {

StepPrinter::StepPrinter(const std::string& name,
                         framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  trackID_ = parameters.getParameter<int>("track_id");
}

StepPrinter::~StepPrinter() {}

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
  auto volume{track->GetVolume()->GetName()};

  // Get the next volume
  auto nextVolume{track->GetNextVolume()->GetName()};

  // Get the region
  auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};

  std::cout << "*******************************\n"
            << "*   Step " << track->GetCurrentStepNumber() << "\n"
            << "********************************\n"
            << "\tEnergy of " << particleName << " : " << energy << "\n"
            << "\tTrack ID: " << track->GetTrackID() << "\n"
            << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
            << "\tParticle currently in " << volume << "\n"
            << "\tRegion: " << region << "\n"
            << "\tNext volume: " << nextVolume << "\n"
            << "********************************\n"
            << std::endl;
}

}  // namespace biasing

DECLARE_ACTION(biasing, StepPrinter)
