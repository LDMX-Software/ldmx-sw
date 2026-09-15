/**
 * @file APrimePhysics.h
 * @brief Class which defines basic APrime physics
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_DARKBREM_APRIMEPHYSICS_H_
#define SIMCORE_DARKBREM_APRIMEPHYSICS_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"
#include "G4DarkBreM/G4APrime.h"
#include "G4DarkBreM/G4DarkBreMModel.h"
#include "G4DarkBreM/G4DarkBremsstrahlung.h"
#include "G4DarkBreM/G4FractionallyCharged.h"
#include "SimCore/APrimeConversionToFCPs.h"
#include "SimCore/G4User/UserEventInformation.h"

// Geant4
#include "G4Electron.hh"
#include "G4EventManager.hh"
#include "G4ProcessManager.hh"
#include "G4VPhysicsConstructor.hh"

namespace simcore {

/**
 * @class APrimePhysics
 * @brief Defines basic APrime physics
 *
 * It constructs the APrime particle and links the dark brem process to the
 * electron.
 *
 * @see G4APrime
 * @see G4DarkBremsstrahlung in G4DarkBreM
 *
 * @note
 * This class basically does not do anything except
 * register the custom particle (G4APrime) and custom
 * process (G4DarkBremsstrahlung).
 */
class APrimePhysics : public G4VPhysicsConstructor {
 public:
  /**
   * The name of this physics constructor.
   *
   * Passed into Geant4 to register this physics,
   * should not conflict with any other Geant4
   * physics.
   */
  static const std::string NAME;

  /**
   * Class constructor.
   *
   * @param params Parameters to configure the dark brem process
   */
  APrimePhysics(const framework::config::Parameters& params);

  /**
   * Class destructor.
   *
   * Nothing right now.
   */
  virtual ~APrimePhysics() = default;

  /**
   * Construct particle.
   *
   * Insert A' into the Geant4 particle table.
   * Geant4 registers all instances derived from G4ParticleDefinition
   * and deletes them at the end of processing.
   *
   * Uses the A' mass given by the parameter APrimeMass to
   * inform the G4APrime instance what mass to use.
   *
   * @see G4APrime
   */
  void ConstructParticle();

  /**
   * Construct the process.
   *
   * Links the dark brem processs to the electron through the process manager
   * only if the dark brem process is enabled ('enable' is True).
   *
   * Also sets up the A' -> fcp+ fcp- conversion process if fcp is enabled.
   *
   * G4ProcessManager registers and cleans up any created processes,
   * so we can forget about it after creating it.
   *
   * @see G4DarkBremsstrahlung in G4DarkBreM
   * @see APrimeConversionToFCPs
   */
  void ConstructProcess();

 private:
  /// the mass of the A' for this run
  G4double ap_mass_;

  /// is dark brem enabled for this run?
  bool enable_;

  /// is A' -> fcp conversion enabled for this run?
  bool fcp_enable_;

  /// mass of the fcp in MeV
  G4double fcp_mass_;

  /// charge of the fcp in units of e
  G4double fcp_charge_;

  /// cross section biasing factor for A' -> fcp conversion
  G4double fcp_xsec_factor_;

  /**
   * Dark brem parameters to pass to the process (if enabled)
   *
   * @note This can't be a reference because we pass it to
   * the process _after_ the configuration step from our POV
   * is done. Thus we need our own copy that won't be destroyed.
   */
  framework::config::Parameters parameters_;

  std::unique_ptr<G4DarkBremsstrahlung> process_;

  /// A' -> fcp conversion process (owned by G4 process manager after
  /// registration)
  APrimeConversionToFCPs* fcp_conversion_process_{nullptr};

  /**
   * Enable logging for this class.
   */
  enableLogging("APrimePhysics")
};

}  // namespace simcore

#endif
