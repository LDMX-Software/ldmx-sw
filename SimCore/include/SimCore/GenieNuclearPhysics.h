/**
 * @file GenieNuclearPhysics.h
 * @brief G4VPhysicsConstructor that registers the GENIE electronuclear
 *        process with the electron.
 */

#ifndef SIMCORE_GENIENUCLEARPHYSICS_H
#define SIMCORE_GENIENUCLEARPHYSICS_H

#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"
#include "G4VPhysicsConstructor.hh"

namespace simcore {

/**
 * @class GenieNuclearPhysics
 * @brief Physics constructor that replaces the built-in Geant4 electronNuclear
 *        process with the GENIE-based GenieElectroNuclearProcess.
 *
 * Follows the same pattern as APrimePhysics.  Registered with the physics
 * list in RunManager::setupPhysics().
 */
class GenieNuclearPhysics : public G4VPhysicsConstructor {
 public:
  /// Name of this physics constructor
  static const std::string NAME;

  /**
   * Constructor.
   * @param params Configuration parameters forwarded to the process
   */
  GenieNuclearPhysics(const framework::config::Parameters& params);

  ~GenieNuclearPhysics() override = default;

  /// No new particles to construct
  void ConstructParticle() override;

  /**
   * Remove the built-in electronNuclear process and register the
   * GENIE-based replacement.
   */
  void ConstructProcess() override;

 private:
  /// Stored configuration to forward to the process
  framework::config::Parameters params_;

  /// Is the GENIE process enabled?
  bool enable_;

  enableLogging("GenieNuclearPhysics")
};

}  // namespace simcore

#endif  // SIMCORE_GENIENUCLEARPHYSICS_H
