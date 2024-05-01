/**
 * @file LHEParticle.h
 * @brief Class defining a single particle record in an LHE event
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_LHEPARTICLE_H_
#define SIMCORE_LHEPARTICLE_H_

// STL
#include <string>
#include <vector>

namespace simcore::lhe {

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
  int getIDUP() const;

  /**
   * Get the status code (ISTUP).
   * @return The status code.
   */
  int getISTUP() const;

  /**
   * Get a mother particle index (MOTHUP) by index.
   * @return The mother particle by index.
   */
  int getMOTHUP(int) const;

  /**
   * Get the particle color (ICOLUP) by index.
   * @return The particle color by index.
   */
  int getICOLUP(int) const;

  /**
   * Get a momentum component (PUP) by index.
   * Defined in order: E/C, Px, Py, Pz, mass
   * @return The momentum component by index.
   */
  double getPUP(int) const;

  /**
   * Get the proper lifetime (VTIMUP).
   * @return The particle's proper lifetime.
   */
  double getVTIMUP() const;

  /**
   * Get the particle's spin (SPINUP).
   * @return The particle's spin.
   */
  double getSPINUP() const;

  /**
   * Set a mother particle by index.
   * @param i The mother index.
   * @param particle The mother particle.
   */
  void setMother(int i, LHEParticle* particle);

  /**
   * Get a mother particle by index.
   * @return The mother particle at the index.
   */
  LHEParticle* getMother(int) const;

  /**
   * Print particle information to an output stream.
   * @param stream The output stream.
   */
  void print(std::ostream& stream) const;

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
  int idup_;

  /**
   * The status code.
   */
  int istup_;

  /**
   * The mother particle indices.
   */
  int mothup_[2];

  /**
   * The particle color.
   */
  int icolup_[2];

  /**
   * The momentum components.
   */
  double pup_[5];

  /**
   * The proper time.
   */
  double vtimup_;

  /**
   * The particle's spin.
   */
  int spinup_;
};

}  // namespace simcore::lhe

#endif
