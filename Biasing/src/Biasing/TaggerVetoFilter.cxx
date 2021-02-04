
#include "Biasing/TaggerVetoFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"
#include "G4Step.hh"

namespace biasing {

TaggerVetoFilter::TaggerVetoFilter(const std::string& name,
                                   framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");
}

TaggerVetoFilter::~TaggerVetoFilter() {}

void TaggerVetoFilter::stepping(const G4Step* step) {
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
  if (auto region{
          track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("tagger") != 0)
    return;

  // If the energy of the particle falls below threshold, stop
  // processing the event.
  if (auto energy{step->GetPostStepPoint()->GetTotalEnergy()};
      energy < threshold_) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }
}

}  // namespace biasing

DECLARE_ACTION(biasing, TaggerVetoFilter)
