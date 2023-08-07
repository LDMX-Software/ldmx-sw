#include "SimCore/TrackMap.h"

// Geant4
#include "G4Event.hh"
#include "G4EventManager.hh"

namespace simcore {

void TrackMap::insert(const G4Track* track) {
  ancestry_[track->GetTrackID()] = std::make_pair(track->GetParentID(),isInCalorimeterRegion(track));
  descendents_[track->GetParentID()].push_back(track->GetTrackID());
}

int TrackMap::findIncident(G4int trackID) const {
  int currTrackID = trackID;
  bool foundIncident{false};
  while (not foundIncident) {
    auto &[parentID, inCalRegion] = ancestry_.at(currTrackID);
    if (not inCalRegion or parentID == 0) {
      // current track ID is nearest ancestor
      // originating outside cal region
      // or is a primary particle
      foundIncident = true;
    } else {
      // still in cal region, keep going
      currTrackID = parentID;
    }
  }
  return currTrackID;
}

void TrackMap::save(const G4Track* track) {
  // create sim particle in map, keep reference to the newly created particle
  ldmx::SimParticle& particle{particle_map_[track->GetTrackID()]};

  // TODO: default gen status?
  particle.setGenStatus(0);

  // Update the gen status from the primary particle.
  if (track->GetDynamicParticle()->GetPrimaryParticle() != nullptr) {
    G4VUserPrimaryParticleInformation* primaryInfo =
        track->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation();
    if (primaryInfo != nullptr) {
      particle.setGenStatus(
          ((UserPrimaryParticleInformation*)primaryInfo)->getHepEvtStatus());
    }
  }

  auto particle_def{track->GetDefinition()};

  particle.setPdgID(particle_def->GetPDGEncoding());
  particle.setCharge(particle_def->GetPDGCharge());
  particle.setMass(track->GetDynamicParticle()->GetMass());
  particle.setEnergy(track->GetVertexKineticEnergy()+track->GetDynamicParticle()->GetMass());

  auto track_info{UserTrackInformation::get(track)};
  particle.setVertexVolume(track_info->getVertexVolume());

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
    particle.setProcessType(ldmx::SimParticle::ProcessType::unknown);
  }

  // track's current kinematics is its end point kinematics
  //  because we are assuming this track is being stopped/killed

  auto momentum{track->GetMomentum()};
  particle.setEndPointMomentum(momentum.x(), momentum.y(), momentum.z());

  auto end_pt{track->GetPosition()};
  particle.setEndPoint(end_pt.x(), end_pt.y(), end_pt.z());
}

void TrackMap::traceAncestry() {
  for (auto & [id, particle] : particle_map_) {
    particle.addParent(ancestry_.at(id).first);
    
    /**
     * Use [] instead of at() for descendents_
     * so that if it wasn't previously created,
     * we will just silently create an empty
     * vector and move on.
     */
    for (auto &child : descendents_[id]) {
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
