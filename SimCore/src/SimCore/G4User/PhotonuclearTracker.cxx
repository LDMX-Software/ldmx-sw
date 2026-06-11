#include "SimCore/G4User/PhotonuclearTracker.h"

#include "G4Element.hh"
#include "G4EventManager.hh"
#include "G4Material.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/G4User/UserEventInformation.h"

namespace simcore {

PhotonuclearTracker* PhotonuclearTracker::instance = nullptr;

PhotonuclearTracker::PhotonuclearTracker(
    const std::string& name, framework::config::Parameters& parameters)
    : UserAction(name, parameters) {
  instance = this;  // Set the static instance pointer
}

void PhotonuclearTracker::BeginOfEventAction(const G4Event* event) {
  // Clear all data structures for the new event
  pn_interactions_.clear();
  secondary_to_pn_index_.clear();
  descendant_ancestry_.clear();
}

void PhotonuclearTracker::EndOfEventAction(const G4Event* event) {
  ldmx_log(debug) << "Event complete, recorded " << pn_interactions_.size()
                  << " PN interactions";
}

void PhotonuclearTracker::stepping(const G4Step* step) {
  // Check if any secondaries were created in this step
  const std::vector<const G4Track*>* secondaries =
      step->GetSecondaryInCurrentStep();
  if (!secondaries || secondaries->empty()) return;

  // Check if any secondary was created by a photonNuclear process
  bool is_pn_interaction = false;
  for (const G4Track* secondary : *secondaries) {
    const G4VProcess* creator = secondary->GetCreatorProcess();
    if (creator) {
      const G4String& creator_name = creator->GetProcessName();
      if (creator_name.contains("photonNuclear")) {
        is_pn_interaction = true;
        break;
      }
    }
  }

  if (!is_pn_interaction) return;

  // We have a PN interaction! Create a new PhotonuclearInteraction object
  ldmx::PhotonuclearInteraction pn_interaction;

  // Get the incident photon information from the track that took the step
  const G4Track* incident_track = step->GetTrack();
  const G4StepPoint* pre_step = step->GetPreStepPoint();

  auto incident_pos = pre_step->GetPosition();
  auto incident_momentum = pre_step->GetMomentum();

  pn_interaction.setIncidentPhoton(
      incident_track->GetTrackID(),
      incident_track->GetDefinition()->GetPDGEncoding(),
      pre_step->GetKineticEnergy() +
          incident_track->GetDefinition()->GetPDGMass(),
      incident_momentum.x(), incident_momentum.y(), incident_momentum.z(),
      incident_pos.x(), incident_pos.y(), incident_pos.z(),
      pre_step->GetGlobalTime());

  // Get target nucleus information
  int target_z, target_a;
  std::string target_material;
  extractTargetInfo(step, target_z, target_a, target_material);
  pn_interaction.setTarget(target_z, target_a, target_material);

  // Set interaction metadata
  G4VPhysicalVolume* volume{incident_track->GetVolume()};
  pn_interaction.setInteractionVolume(volume ? volume->GetName() : "null");
  pn_interaction.setProcessName("photonNuclear");

  // Record all immediate secondaries
  int pn_index = pn_interactions_.size();

  // Set target Z/A in UserEventInformation for the first PN interaction
  if (pn_index == 0) {
    auto event_info = static_cast<UserEventInformation*>(
        G4EventManager::GetEventManager()->GetUserInformation());
    if (event_info) {
      event_info->setPNTargetZ(target_z);
      event_info->setPNTargetA(target_a);
    }
  }
  for (const G4Track* secondary : *secondaries) {
    auto particle_info = createParticleInfo(secondary);
    pn_interaction.addImmediateSecondary(particle_info);

    // Map this secondary to the current PN interaction
    int secondary_id = secondary->GetTrackID();
    secondary_to_pn_index_[secondary_id] = pn_index;
    // Initialize ancestry: immediate secondaries map to themselves
    descendant_ancestry_[secondary_id] = secondary_id;

    ldmx_log(debug) << "  Immediate secondary: track_id=" << secondary_id
                    << " pdg=" << particle_info.pdg_id_
                    << " E=" << particle_info.energy_ << " MeV";
  }

  // Add the completed PN interaction to the collection
  pn_interactions_.push_back(pn_interaction);

  ldmx_log(debug) << "Detected PN interaction #" << pn_index << " with "
                  << secondaries->size() << " secondaries";
}

void PhotonuclearTracker::PreUserTrackingAction(const G4Track* track) {
  // Propagate ancestry information to new tracks
  int track_id = track->GetTrackID();
  int parent_id = track->GetParentID();

  // If the parent is a descendant of a PN secondary, this track is too
  auto parent_it = descendant_ancestry_.find(parent_id);
  if (parent_it != descendant_ancestry_.end()) {
    // Propagate the immediate secondary ID to this descendant
    descendant_ancestry_[track_id] = parent_it->second;

    ldmx_log(debug) << "  Track " << track_id
                    << " is descendant of PN secondary " << parent_it->second
                    << " (parent=" << parent_id << ")";
  }
}

void PhotonuclearTracker::PostUserTrackingAction(const G4Track* track) {
  int track_id = track->GetTrackID();

  // Check if this track is a descendant of a PN secondary
  auto ancestry_it = descendant_ancestry_.find(track_id);
  if (ancestry_it == descendant_ancestry_.end()) {
    return;  // Not related to any PN interaction
  }

  // This track is a descendant! Find which immediate secondary it came from
  int immediate_secondary_id = ancestry_it->second;

  // Find which PN interaction this secondary belongs to
  auto pn_index_it = secondary_to_pn_index_.find(immediate_secondary_id);
  if (pn_index_it == secondary_to_pn_index_.end()) {
    ldmx_log(warn) << "Inconsistent ancestry mapping for track " << track_id;
    return;
  }

  int pn_index = pn_index_it->second;

  // Record ALL descendants (intermediate + final state) for full cascade
  // genealogy This provides information beyond SimParticles which only has
  // final state
  pn_interactions_[pn_index].addDescendant(immediate_secondary_id, track_id);

  ldmx_log(debug) << "  Recording descendant: track " << track_id
                  << " -> secondary " << immediate_secondary_id
                  << " in PN interaction " << pn_index;
}

void PhotonuclearTracker::extractTargetInfo(const G4Step* step, int& Z, int& A,
                                            std::string& material) {
  const G4Material* mat = step->GetPreStepPoint()->GetMaterial();
  material = mat->GetName();

  // Get the element - for compound materials, we take the first element
  // (This is a simplification; the actual nucleus struck may vary)
  const G4Element* element = mat->GetElement(0);
  Z = static_cast<int>(element->GetZ());
  A = static_cast<int>(element->GetA() /
                       (CLHEP::g / CLHEP::mole));  // Convert to mass number
}

ldmx::PhotonuclearInteraction::ParticleInfo
PhotonuclearTracker::createParticleInfo(const G4Track* track) {
  ldmx::PhotonuclearInteraction::ParticleInfo info;

  info.track_id_ = track->GetTrackID();
  info.pdg_id_ = track->GetDefinition()->GetPDGEncoding();

  auto position = track->GetPosition();
  info.x_ = position.x();
  info.y_ = position.y();
  info.z_ = position.z();

  auto momentum = track->GetMomentum();
  info.px_ = momentum.x();
  info.py_ = momentum.y();
  info.pz_ = momentum.z();

  info.energy_ =
      track->GetKineticEnergy() + track->GetDefinition()->GetPDGMass();
  info.time_ = track->GetGlobalTime();

  return info;
}

}  // namespace simcore

DECLARE_ACTION(simcore::PhotonuclearTracker)
