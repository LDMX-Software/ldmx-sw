/**
 * @file GenieBeamElectronKiller.cxx
 * @brief Implementation of the GenieBeamElectronKiller stepping action.
 */

#include "SimCore/G4User/GenieBeamElectronKiller.h"

#include "G4Step.hh"
#include "G4Track.hh"
#include "SimCore/G4User/PtrRetrieval.h"

namespace simcore {

GenieBeamElectronKiller::GenieBeamElectronKiller(
    const std::string& name, framework::config::Parameters& parameters)
    : UserAction(name, parameters) {}

void GenieBeamElectronKiller::stepping(const G4Step* step) {
  auto track = step->GetTrack();

  // Only process the primary electron (parentID == 0, PDG == 11)
  if (track->GetParentID() != 0) return;
  if (track->GetParticleDefinition()->GetPDGEncoding() != 11) return;

  // Check if the track is in the target region
  static auto target_region =
      simcore::g4user::ptrretrieval::getRegion("target");
  if (!target_region) return;

  auto phy_vol = track->GetVolume();
  auto log_vol = phy_vol ? phy_vol->GetLogicalVolume() : nullptr;
  auto track_region = log_vol ? log_vol->GetRegion() : nullptr;

  if (track_region == target_region) {
    track->SetTrackStatus(fStopAndKill);
    ldmx_log(debug) << "Killed beam electron at target region";
  }
}

}  // namespace simcore

DECLARE_ACTION(simcore::GenieBeamElectronKiller)
