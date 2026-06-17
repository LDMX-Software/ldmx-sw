#include "SimCore/G4User/TrackMap.h"

// Geant4
#include "G4Event.hh"
#include "G4EventManager.hh"

namespace simcore {

bool TrackMap::isDescendant(int trackID, int ancestorID,
                            int maximum_depth) const {
  int current_depth{0};
  int current_track{trackID};
  // Walk the tree until we either no longer have a parent or we reach the
  // desired depth
  while (current_depth < maximum_depth &&
         (ancestry_.find(current_track) != ancestry_.end())) {
    // See if we have encountered the parent of the current track
    //
    // operator[] is not const, so we need to use at()
    current_track = ancestry_.at(current_track).first;
    if (current_track == ancestorID) {
      // If one of the parents is the track of interest, we are done!
      return true;
    }
    current_depth++;
  }
  return false;
}
void TrackMap::insert(const G4Track* track) {
  ancestry_[track->GetTrackID()] =
      std::make_pair(track->GetParentID(), isInCalorimeterRegion(track));
  descendents_[track->GetParentID()].push_back(track->GetTrackID());
}

int TrackMap::findIncident(G4int trackID) const {
  int curr_track_id = trackID;
  bool found_incident{false};
  while (not found_incident) {
    auto& [parentID, inCalRegion] = ancestry_.at(curr_track_id);
    if (not inCalRegion or parentID == 0) {
      // current track ID is nearest ancestor
      // originating outside cal region
      // or is a primary particle
      found_incident = true;
    } else {
      // still in cal region, keep going
      curr_track_id = parentID;
    }
  }
  return curr_track_id;
}

void TrackMap::save(const G4Track* track) {
  // create sim particle in map, keep reference to the newly created particle
  ldmx::SimParticle& particle{particle_map_[track->GetTrackID()]};

  // TODO: default gen status?
  particle.setGenStatus(0);

  // Update the gen status from the primary particle.
  if (track->GetDynamicParticle()->GetPrimaryParticle() != nullptr) {
    G4VUserPrimaryParticleInformation* primary_info =
        track->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation();
    if (primary_info != nullptr) {
      particle.setGenStatus(
          ((UserPrimaryParticleInformation*)primary_info)->getHepEvtStatus());
    }
  }

  auto particle_def{track->GetDefinition()};

  particle.setPdgID(particle_def->GetPDGEncoding());
  particle.setCharge(particle_def->GetPDGCharge());
  particle.setMass(track->GetDynamicParticle()->GetMass());
  particle.setEnergy(track->GetVertexKineticEnergy() +
                     track->GetDynamicParticle()->GetMass());

  auto track_info{UserTrackInformation::get(track)};
  particle.setVertexVolume(track_info->getVertexVolume());
  particle.setInteractionMaterial(track->GetMaterial()->GetName());

  auto vert{track->GetVertexPosition()};
  particle.setVertex(vert.x(), vert.y(), vert.z());
  particle.setTime(track_info->getVertexTime());

  auto init_momentum{track_info->getInitialMomentum()};
  particle.setMomentum(init_momentum.x(), init_momentum.y(), init_momentum.z());

  const G4VProcess* process{track->GetCreatorProcess()};
  if (process) {
    const G4String& name{process->GetProcessName()};
    particle.setProcessType(ldmx::SimParticle::findProcessType(name));
  } else {
    if (track->GetParentID() == 0) {
      // genStatus==2 indicates a GENIE EN product generated alongside
      // an upstream beam electron; label these as electronNuclear
      if (particle.getGenStatus() == 2) {
        particle.setProcessType(
            ldmx::SimParticle::ProcessType::electronNuclear);
      } else {
        particle.setProcessType(ldmx::SimParticle::ProcessType::Primary);
      }
    } else {
      particle.setProcessType(ldmx::SimParticle::ProcessType::unknown);
    }
  }

  // track's current kinematics is its end point kinematics
  //  because we are assuming this track is being stopped/killed

  auto momentum{track->GetMomentum()};
  particle.setEndPointMomentum(momentum.x(), momentum.y(), momentum.z());

  auto end_pt{track->GetPosition()};
  particle.setEndPoint(end_pt.x(), end_pt.y(), end_pt.z());
}

void TrackMap::traceAncestry() {
  // When GENIE is used with an upstream beam electron, the GENIE EN products
  // (genStatus==2) should be parented to the beam electron (genStatus==1,
  // PDG==11) rather than having parentID 0 (no parent).
  int beam_electron_id = 0;
  bool has_genie_products = false;
  for (auto& [id, particle] : particle_map_) {
    if (particle.getGenStatus() == 2) {
      has_genie_products = true;
    }
    if (beam_electron_id == 0 && particle.getGenStatus() == 1 &&
        particle.getPdgID() == 11 && ancestry_.at(id).first == 0) {
      beam_electron_id = id;
    }
  }

  for (auto& [id, particle] : particle_map_) {
    int parent_id = ancestry_.at(id).first;

    // Re-parent GENIE EN products to the beam electron
    if (has_genie_products && particle.getGenStatus() == 2 &&
        beam_electron_id > 0 && parent_id == 0) {
      particle.addParent(beam_electron_id);
      particle_map_[beam_electron_id].addDaughter(id);
    } else {
      particle.addParent(parent_id);
    }

    /**
     * Use [] instead of at() for descendents_
     * so that if it wasn't previously created,
     * we will just silently create an empty
     * vector and move on.
     */
    for (auto& child : descendents_[id]) {
      particle.addDaughter(child);
    }
  }
}

void TrackMap::clear() {
  ancestry_.clear();
  descendents_.clear();
  particle_map_.clear();
}

bool TrackMap::isInCalorimeterRegion(const G4Track* track) const {
  auto region{track->GetLogicalVolumeAtVertex()->GetRegion()->GetName()};
  return region.contains("Calorimeter");
}

}  // namespace simcore
