/**
 * @file HepMCParticle.h
 * @brief Class defining a single particle record in a HepMC event
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_HEPMCPARTICLE_H_
#define SIMCORE_HEPMCPARTICLE_H_

// LDMX
#include "Framework/Exception/Exception.h"

// HepMC3
#include "HepMC3/GenParticle.h"
#include "HepMC3/FourVector.h"

// STL
#include <iostream>
#include <memory>

// Geant4
#include "globals.hh"

namespace simcore {
namespace hepmc {

/**
 * @class HepMCParticle
 * @brief Wrapper class for HepMC3::GenParticle
 */
class HepMCParticle {
 public:
  /**
   * Class constructor.
   * @param particle Shared pointer to HepMC3 GenParticle
   */
  HepMCParticle(std::shared_ptr<HepMC3::GenParticle> particle);

  /**
   * Get the PDG code.
   * @return The PDG code.
   */
  int getPdgId() const;

  /**
   * Get the status code.
   * @return The status code.
   */
  int getStatus() const;

  /**
   * Get a momentum component by index.
   * Index 0: px, 1: py, 2: pz, 3: E
   * @return The momentum component by index.
   */
  double getMomentum(int index) const;

  /**
   * Get the particle's mass.
   * @return The particle's mass.
   */
  double getMass() const;

  /**
   * Get the underlying HepMC3 GenParticle
   * @return Shared pointer to the HepMC3 GenParticle
   */
  std::shared_ptr<HepMC3::GenParticle> getGenParticle() const;

  /**
   * Print particle information to an output stream.
   */
  void print() const;

  /**
   * Overloaded stream operator.
   * @param stream The output stream.
   * @param particle The particle to print.
   */
  friend std::ostream& operator<<(std::ostream& stream,
                                  const HepMCParticle& particle);

 private:
  /**
   * The underlying HepMC3 GenParticle
   */
  std::shared_ptr<HepMC3::GenParticle> particle_;
};

}  // namespace hepmc
}  // namespace simcore

#endif
