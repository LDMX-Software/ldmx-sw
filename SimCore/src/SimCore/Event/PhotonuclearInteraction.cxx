#include "SimCore/Event/PhotonuclearInteraction.h"

#include <iostream>

ClassImp(ldmx::PhotonuclearInteraction);
ClassImp(ldmx::PhotonuclearInteraction::ParticleInfo);

namespace ldmx {

void PhotonuclearInteraction::clear() {
  incident_photon_ = ParticleInfo();
  target_z_ = 0;
  target_a_ = 0;
  target_material_ = "";
  immediate_secondaries_.clear();
  descendant_map_.clear();
  interaction_volume_ = "";
  process_name_ = "";
}

void PhotonuclearInteraction::setIncidentPhoton(int track_id, int pdg_id,
                                                double energy, double px,
                                                double py, double pz, double x,
                                                double y, double z,
                                                double time) {
  incident_photon_.track_id_ = track_id;
  incident_photon_.pdg_id_ = pdg_id;
  incident_photon_.energy_ = energy;
  incident_photon_.px_ = px;
  incident_photon_.py_ = py;
  incident_photon_.pz_ = pz;
  incident_photon_.x_ = x;
  incident_photon_.y_ = y;
  incident_photon_.z_ = z;
  incident_photon_.time_ = time;
}

void PhotonuclearInteraction::setTarget(int Z, int A,
                                        const std::string& material) {
  target_z_ = Z;
  target_a_ = A;
  target_material_ = material;
}

void PhotonuclearInteraction::addImmediateSecondary(
    const ParticleInfo& particle) {
  immediate_secondaries_.push_back(particle);
}

void PhotonuclearInteraction::addDescendant(int immediate_secondary_id,
                                            int final_state_track_id) {
  descendant_map_[immediate_secondary_id].push_back(final_state_track_id);
}

std::vector<int> PhotonuclearInteraction::getDescendants(
    int immediate_secondary_id) const {
  auto it = descendant_map_.find(immediate_secondary_id);
  if (it != descendant_map_.end()) {
    return it->second;
  }
  return std::vector<int>();
}

std::ostream& operator<<(std::ostream& o, const PhotonuclearInteraction& pn) {
  o << "PhotonuclearInteraction:" << "  Process: " << pn.process_name_
    << "  Volume: " << pn.interaction_volume_
    << "  Incident photon: track_id=" << pn.incident_photon_.track_id_
    << " pdg=" << pn.incident_photon_.pdg_id_
    << " E=" << pn.incident_photon_.energy_ << " MeV"
    << "  Target: Z=" << pn.target_z_ << " A=" << pn.target_a_
    << " material=" << pn.target_material_
    << "  Immediate secondaries: " << pn.immediate_secondaries_.size()
    << std::endl;
  for (const auto& sec : pn.immediate_secondaries_) {
    o << "    track_id=" << sec.track_id_ << " pdg=" << sec.pdg_id_
      << " E=" << sec.energy_ << " MeV" << std::endl;
  }
  o << "  Descendant map entries: " << pn.descendant_map_.size() << std::endl;
  for (const auto& [sec_id, descendants] : pn.descendant_map_) {
    o << "    Secondary " << sec_id << " -> " << descendants.size()
      << " descendants" << std::endl;
  }
  return o;
}

void PhotonuclearInteraction::print() const { std::cout << *this; }

}  // namespace ldmx
