#include "SimCore/HepMC/HepMCParticle.h"

#include <iostream>

namespace simcore {
namespace hepmc {

HepMCParticle::HepMCParticle(std::shared_ptr<HepMC3::GenParticle> particle)
    : particle_(particle) {
  if (!particle_) {
    EXCEPTION_RAISE("NullParticle",
                    "Attempted to create HepMCParticle with null GenParticle");
  }
}

int HepMCParticle::getPdgId() const { return particle_->pid(); }

int HepMCParticle::getStatus() const { return particle_->status(); }

double HepMCParticle::getMomentum(int index) const {
  const HepMC3::FourVector& momentum = particle_->momentum();
  switch (index) {
    case 0:
      return momentum.px();
    case 1:
      return momentum.py();
    case 2:
      return momentum.pz();
    case 3:
      return momentum.e();
    default:
      EXCEPTION_RAISE(
          "InvalidIndex",
          "Momentum index must be 0 (px), 1 (py), 2 (pz), or 3 (E)");
  }
}

double HepMCParticle::getMass() const { return particle_->generated_mass(); }

std::shared_ptr<HepMC3::GenParticle> HepMCParticle::getGenParticle() const {
  return particle_;
}

void HepMCParticle::print() const { std::cout << *this << std::endl; }

std::ostream& operator<<(std::ostream& stream, const HepMCParticle& particle) {
  stream << "HepMCParticle: " << "PDG=" << particle.getPdgId()
         << " Status=" << particle.getStatus()
         << " px=" << particle.getMomentum(0)
         << " py=" << particle.getMomentum(1)
         << " pz=" << particle.getMomentum(2)
         << " E=" << particle.getMomentum(3) << " mass=" << particle.getMass();
  return stream;
}

}  // namespace hepmc
}  // namespace simcore
