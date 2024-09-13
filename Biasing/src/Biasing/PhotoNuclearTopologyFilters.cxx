#include "Biasing/PhotoNuclearTopologyFilters.h"

namespace biasing {

bool NothingHardFilter::rejectEvent(
    const std::vector<G4Track*>& secondaries) const {
  for (const auto& secondary : secondaries) {
    // Get the PDG ID of the track
    const auto pdgID{
        std::abs(secondary->GetParticleDefinition()->GetPDGEncoding())};
    if (skipCountingParticle(pdgID)) {
      continue;
    }
    auto energy{secondary->GetKineticEnergy()};
    if (energy > hard_particle_threshold_) {
      return true;
    }
  }
  return false;
}
bool SingleNeutronFilter::rejectEvent(
    const std::vector<G4Track*>& secondaries) const {
  int hard_particles{0};
  int hard_neutrons{0};
  for (const auto& secondary : secondaries) {
    // Get the PDG ID of the track
    const auto pdgID{
        std::abs(secondary->GetParticleDefinition()->GetPDGEncoding())};
    if (skipCountingParticle(pdgID)) {
      continue;
    }
    auto energy{secondary->GetKineticEnergy()};
    if (energy > hard_particle_threshold_) {
      hard_particles++;
      if (isNeutron(pdgID)) {
        hard_neutrons++;
      }
    }
  }
  auto reject{hard_particles != hard_neutrons || hard_particles != 1};
  return reject;
}

PhotoNuclearTopologyFilter::PhotoNuclearTopologyFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : UserAction{name, parameters},
      count_light_ions_{parameters.getParameter<bool>("count_light_ions")},
      hard_particle_threshold_{
          parameters.getParameter<double>("hard_particle_threshold")} {}

void PhotoNuclearTopologyFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Get the track info and check if this track has been tagged as the
  // photon that underwent a photo-nuclear reaction. Only those tracks
  // tagged as PN photos will be processed. The track is currently only
  // tagged by the UserAction ECalProcessFilter which needs to be run
  // before this UserAction.
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  if ((trackInfo != nullptr) && !trackInfo->isPNGamma()) return;

  // Get the PN photon daughters.
  auto secondaries{step->GetSecondary()};

  if (rejectEvent(*secondaries)) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
  }

  // Once the PN gamma has been procesed, untag it so its not reprocessed
  // again.
  trackInfo->tagPNGamma(false);
}

}  // namespace biasing

DECLARE_ACTION(biasing, NothingHardFilter)
DECLARE_ACTION(biasing, SingleNeutronFilter)
