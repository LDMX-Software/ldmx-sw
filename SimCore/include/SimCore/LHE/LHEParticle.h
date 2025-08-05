/**
 * @file LHEParticle.h
 * @brief Class defining a single particle record in an LHE event
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_LHEPARTICLE_H_
#define SIMCORE_LHEPARTICLE_H_

// LDMX
#include "Framework/Exception/Exception.h"

// STL
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Geant4
#include "globals.hh"

namespace simcore {
namespace lhe {

/**
 * @class LHEParticle
 * @brief Single particle record in an LHE event
 */
class LHEParticle {
 public:
  /**
   * Class constructor.
   * @param data The particle record as a space-delimited string.
   */
  LHEParticle(std::string& data);

  /**
   * Get the PDG code (IDUP).
   * @return The PDG code.
   */
  int getPdgId() const;

  /**
   * Get the status code (ISTUP).
   * @return The status code.
   */
  int getStatus() const;

  /**
   * Get a mother particle index_ (MOTHUP) by index_.
   * @return The mother particle by index_.
   */
  int getMother(int) const;

  /**
   * Get the particle color (ICOLUP) by index_.
   * @return The particle color by index_.
   */
  int getColor(int) const;

  /**
   * Get a momentum component (PUP) by index_.
   * Defined in order: E/C, Px, Py, Pz, mass
   * @return The momentum component by index_.
   */
  double getMomentum(int) const;

  /**
   * Get the proper lifetime (VTIMUP).
   * @return The particle's proper lifetime.
   */
  double getLifetime() const;

  /**
   * Get the particle's spin (SPINUP).
   * @return The particle's spin.
   */
  double getSpin() const;

  /**
   * Set a mother particle by index_.
   * @param i The mother index_.
   * @param particle The mother particle.
   */
  void setMother(int i, LHEParticle* particle);

  /**
   * Get a mother particle by index_.
   * @return The mother particle at the index_.
   */
  LHEParticle* getMotherParticle(int) const;

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
                                  const LHEParticle& particle);

 private:
  /**
   * The mother particles.
   */
  LHEParticle* mothers_[2];

  /**
   * The PDG code.
   */
  int pdg_id_;

  /**
   * The status code.
   */
  int status_;

  /**
   * The mother particle indices.
   */
  int mother_[2];

  /**
   * The particle color.
   */
  int color_[2];

  /**
   * The momentum components.
   */
  double momentum_[5];

  /**
   * The proper time.
   */
  double lifetime_;

  /**
   * The particle's spin.
   */
  int spin_;
};

}  // namespace lhe
}  // namespace simcore

#endif
