
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

void StepPrinter::stepping(const G4Step* step) {
  // Get the track associated with this step
  auto track{step->GetTrack()};

  const auto track_id{track->GetTrackID()};
  const auto parent{track->GetParentID()};
  // Don't bother filling the map if we aren't going to use it
  if (depth_ > 0) {
    trackParents_[track_id] = parent;
  }

  auto process{track->GetCreatorProcess()};
  std::string process_name{process ? process->GetProcessName() : "Primary"};
  // Unwrap biasing part of process name if present
  if (process_name.find("biasWrapper") != std::string::npos) {
    std::size_t pos = process_name.find_first_of("(") + 1;
    process_name = process_name.substr(pos, process_name.size() - pos - 1);
  }

  // This could be a negated condition, but it is easier to read this way
  //
  auto track_map{simcore::g4user::TrackingAction::get()->getTrackMap()};
  if (track_id == trackID_ ||  // We are the track of interest
      track_map.isDescendant(
          track_id, trackID_,
          depth_) ||  // We are a descendent of the track of interest
      process_name ==
          processName_  // The parent process was the process of interest
  ) {
    // This is an interesting track -> Carry on processing
  } else {
    return;
  }
  // Get the particle name.
  const auto particle_name{track->GetParticleDefinition()->GetParticleName()};

  // Get the energy of the particle
  const auto energy{step->GetPostStepPoint()->GetTotalEnergy()};

  // Get the volume the particle is in.
  auto volume{track->GetVolume()};
  auto volume_name{volume ? volume->GetName() : "undefined"};

  // Get the next volume (can fail if current volume is WorldPV and next is
  // outside the world)
  auto next_volume_ptr{track->GetNextVolume()};
  auto next_volume{next_volume_ptr ? next_volume_ptr->GetName() : "undefined"};

  // Get the region
  G4String region_name{"undefined"};
  if (volume) {
    auto lv{volume->GetLogicalVolume()};
    if (lv) {
      auto region{lv->GetRegion()};
      if (region) {
        region_name = region->GetName();
      }
    }
  }

  std::cout << " Step " << track->GetCurrentStepNumber() << " ("
            << track->GetParticleDefinition()->GetParticleName() << ") {"
            << " Energy: " << energy << " Track ID: " << track->GetTrackID()
            << " Particle currently in: " << volume_name
            << " Region: " << region_name << " Next volume: " << next_volume
            << " Weight: " << track->GetWeight() << " Parent: " << parent
            << " (" << process_name << ") " << " Children:";
  for (auto const& child : *(step->GetSecondaryInCurrentStep())) {
    std::cout << " (" << child->GetTotalEnergy()
              << "): " << child->GetParticleDefinition()->GetPDGEncoding();
  }

  std::cout << " }" << std::endl;
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility::StepPrinter)
