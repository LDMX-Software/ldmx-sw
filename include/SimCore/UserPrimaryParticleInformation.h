/**
 * @file UserPrimaryParticleInformation.h
 * @brief Class that provides extra information for Geant4 primary particles
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USERPRIMARYPARTICLEINFORMATION_H_
#define SIMCORE_USERPRIMARYPARTICLEINFORMATION_H_

// Geant4
#include "G4VUserPrimaryParticleInformation.hh"

namespace simcore {

/**
 * @class UserPrimaryParticleInformation
 * @brief Defines extra information attached to a Geant4 primary particle
 */
class UserPrimaryParticleInformation
    : public G4VUserPrimaryParticleInformation {
 public:
  /**
   * Class Constructor.
   */
  UserPrimaryParticleInformation() = default;

  /**
   * Class destructor.
   */
  virtual ~UserPrimaryParticleInformation() = default;

  /**
   * Set the HEP event status (generator status) e.g. from an LHE particle.
   * @param hepEvtStatus The HEP event status.
   */
  void setHepEvtStatus(int hepEvtStatus) { hepEvtStatus_ = hepEvtStatus; }

  /**
   * Get the HEP event status.
   * @return The HEP event status.
   */
  int getHepEvtStatus() { return hepEvtStatus_; }

  /**
   * Implement virtual method (no-op).
   */
  void Print() const {}

 private:
  /**
   * The HEP event status.
   */
  int hepEvtStatus_{-1};
};

}  // namespace simcore

#endif
