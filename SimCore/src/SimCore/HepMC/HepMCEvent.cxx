#include "SimCore/HepMC/HepMCEvent.h"

namespace simcore {
namespace hepmc {

HepMCEvent::HepMCEvent(std::shared_ptr<HepMC3::GenEvent> event)
    : event_(event) {
  if (!event_) {
    EXCEPTION_RAISE("NullEvent", "Attempted to create HepMCEvent with null GenEvent");
  }

  // Extract vertex information from the signal process vertex
  // or the first vertex if no signal process is defined
  std::shared_ptr<HepMC3::GenVertex> vertex;

  // Try to get the signal process vertex first
  if (event_->vertices().size() > 0) {
    vertex = event_->vertices()[0];
  }

  if (vertex) {
    // HepMC3 uses mm for length and mm/c for time by default
    const HepMC3::FourVector& pos = vertex->position();
    vtx_[0] = pos.x();  // mm
    vtx_[1] = pos.y();  // mm
    vtx_[2] = pos.z();  // mm
    vtxt_ = pos.t();    // mm/c, needs conversion to ns for Geant4

    // Convert time from mm/c to ns: t[ns] = t[mm/c] / c[mm/ns]
    // c = 299.792458 mm/ns
    vtxt_ = vtxt_ / 299.792458;
  } else {
    vtx_[0] = 0.0;
    vtx_[1] = 0.0;
    vtx_[2] = 0.0;
    vtxt_ = 0.0;
  }
}

int HepMCEvent::getNumParticles() const {
  if (!particles_extracted_) {
    extractParticles();
  }
  return particles_.size();
}

double HepMCEvent::getEventWeight() const {
  // HepMC3 events can have multiple weights, we take the first one
  const std::vector<double>& weights = event_->weights();
  return weights.empty() ? 1.0 : weights[0];
}

const double* HepMCEvent::getVertex() const {
  return vtx_;
}

double HepMCEvent::getVertexTime() const {
  return vtxt_;
}

const std::vector<std::unique_ptr<HepMCParticle>>& HepMCEvent::getParticles() const {
  if (!particles_extracted_) {
    extractParticles();
  }
  return particles_;
}

std::shared_ptr<HepMC3::GenEvent> HepMCEvent::getGenEvent() const {
  return event_;
}

void HepMCEvent::extractParticles() const {
  particles_.clear();

  // Get all particles from the event
  for (const auto& particle : event_->particles()) {
    // In HepMC3, status code 1 typically means final state particle
    // Status codes vary by generator, but generally:
    // status = 1: final state particle
    // status = 2: intermediate/decayed particle
    // status > 2: generator-specific codes

    // We include particles with status code 1 (final state)
    if (particle->status() == 1) {
      particles_.push_back(std::make_unique<HepMCParticle>(particle));
    }
  }

  particles_extracted_ = true;
}

}  // namespace hepmc
}  // namespace simcore
