
#include "Biasing/TaggerHitFilter.h"

//~~ Geant4 ~~//
#include "G4RunManager.hh"
#include "G4Step.hh"

namespace biasing {

TaggerHitFilter::TaggerHitFilter(const std::string& name,
                                 framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {}

void TaggerHitFilter::stepping(const G4Step* step) {
  // Get the track associated with this step
  auto track{step->GetTrack()};

  // Only process the primary electron track
  if (track->GetParentID() != 0) return;

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11)
    return;

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the tagger region.
  auto volume{track->GetVolume()}; 
  if (auto region{
          volume->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("tagger") != 0)
    return;
    
}
}  // namespace biasing

DECLARE_ACTION(biasing, TaggerHitFilter)
