/**
 * @file FCPPhysics.h
 * @brief Physics constructor for fractionally charged particle (FCP) processes
 *
 * FCP particle construction and
 * gamma -> fcp+ fcp- conversion on SM photons.
 */

#ifndef SIMCORE_FCPPHYSICS_H_
#define SIMCORE_FCPPHYSICS_H_

#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"
#include "G4DarkBreM/G4FractionallyCharged.h"
#include "G4Gamma.hh"
#include "G4ProcessManager.hh"
#include "G4VPhysicsConstructor.hh"
#include "SimCore/GammaConversionToFCPs.h"

namespace simcore {

/**
 * @class FCPPhysics
 * @brief Physics constructor for fractionally charged particles
 *
 * Handles:
 *  1. Constructing fcp+/fcp- particles via G4FractionallyCharged
 *  2. Registering gamma -> fcp+ fcp- conversion on the photon
 *
 * This is independent of APrimePhysics and GammaPhysics.
 */
class FCPPhysics : public G4VPhysicsConstructor {
 public:
  static const std::string NAME;

  FCPPhysics(const G4String& name,
             const framework::config::Parameters& parameters);

  virtual ~FCPPhysics() = default;

  /**
   * Construct fcp+/fcp- particles and register them in the Geant4
   * particle table.
   */
  void ConstructParticle() override;

  /**
   * Register gamma -> fcp+ fcp- conversion process on the photon
   * if enabled.
   */
  void ConstructProcess() override;

 private:
  /// is this physics constructor enabled?
  bool enable_;

  /// mass of the fcp in MeV
  G4double fcp_mass_;

  /// charge of the fcp in units of e
  G4double fcp_charge_;

  /// PDG ID for the fcp particles
  int fcp_pdg_id_;

  /// gamma -> fcp conversion process (owned by G4 process manager after
  /// registration)
  GammaConversionToFCPs* gamma_fcp_process_{nullptr};

  enableLogging("FCPPhysics")
};

}  // namespace simcore

#endif
