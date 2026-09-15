/**
 * @file APrimeConversionToFCPs.h
 * @brief A' -> fcp+ fcp- conversion in a nuclear field.
 *
 * Cross section parametrization and kinematics adapted from
 * G4GammaConversionToMuons class
 *
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_APRIMECONVERSIONTOFCPS_H_
#define SIMCORE_APRIMECONVERSIONTOFCPS_H_

#include <vector>

#include "Framework/Logger.h"
#include "G4ParticleDefinition.hh"
#include "G4VDiscreteProcess.hh"

namespace simcore {

/**
 * @class APrimeConversionToFCPs
 * @brief Discrete process for A' -> fcp+ fcp- conversion in a nuclear field.
 *
 * This process is the dark-sector analogue of G4GammaConversionToMuons
 * (gamma -> mu+ mu-). A practically massless dark photon (A') converts
 * into a pair of massive fractionally charged particles (fcp+ fcp-)
 * in the electromagnetic field of a nucleus.
 */
class APrimeConversionToFCPs : public G4VDiscreteProcess {
 public:
  /**
   * The name of this process.
   * Used to attach biasing operators to this process.
   */
  static const std::string PROCESS_NAME;

  /**
   * Constructor
   *
   * @param[in] processName name of this process (default: "APrimeToFCPPair")
   * @param[in] type Geant4 process type
   */
  explicit APrimeConversionToFCPs(
      const G4String& processName = "APrimeToFCPPair",
      G4ProcessType type = fElectromagnetic);

  /**
   * Destructor
   * Just the default one
   */
  ~APrimeConversionToFCPs() = default;

  /**
   * Check if this process applies to the given particle.
   * @return true only for the A' (dark photon)
   */
  G4bool IsApplicable(const G4ParticleDefinition& particle) override;

  /**
   * Build physics table (called during initialization).
   * Sets up the temp vector for element selection.
   */
  void BuildPhysicsTable(const G4ParticleDefinition& p) override;

  /**
   * Print process information.
   */
  void printInfoDefinition();

  /**
   * Set a factor to artificially scale the cross section.
   * @param[in] fac multiplicative factor (default 1.0)
   */
  void setCrossSecFactor(G4double fac);

  /** @return the current cross section scaling factor */
  inline G4double getCrossSecFactor() const { return cross_sec_factor_; }

  /**
   * Return the mean free path of the A' in the current material.
   */
  G4double GetMeanFreePath(const G4Track& aTrack, G4double previousStepSize,
                           G4ForceCondition* condition) override;

  /**
   * Return the total cross section per atom.
   */
  G4double getCrossSectionPerAtom(const G4DynamicParticle* aDynamicAPrime,
                                  const G4Element* anElement);

  /**
   * Generate the final state: A' -> fcp+ fcp-
   */
  G4VParticleChange* PostStepDoIt(const G4Track& aTrack,
                                  const G4Step& aStep) override;

  /**
   * Compute the cross section per atom for the given A' energy and Z.
   */
  G4double computeCrossSectionPerAtom(G4double APrimeEnergy, G4int Z);

  /**
   * Compute the mean free path for the given energy and material.
   */
  G4double computeMeanFreePath(G4double APrimeEnergy,
                               const G4Material* aMaterial);

  // delete copy constructor and assignment
  APrimeConversionToFCPs(const APrimeConversionToFCPs&) = delete;
  APrimeConversionToFCPs& operator=(const APrimeConversionToFCPs&) = delete;

 private:
  /**
   * Select a random atom from the material, weighted by cross section.
   */
  const G4Element* selectRandomAtom(const G4DynamicParticle* aDynamicAPrime,
                                    const G4Material* aMaterial);

  /// fcp mass
  G4double mfcp_;
  /// Coupling-scaled reduced Compton wavelength: q^2 * elm_coupling / Mfcp
  G4double rc_;
  /// Energy limit for accurate cross section (5 * Mfcp)
  G4double limit_energy_;
  /// Absolute low energy limit of the model (2 * Mfcp)
  G4double lowest_energy_limit_;
  /// High energy limit of the model
  G4double highest_energy_limit_;
  /// Factor to artificially scale the cross section
  G4double cross_sec_factor_ = 1.0;

  /// Pointer to the A' particle definition
  const G4ParticleDefinition* the_a_prime_;
  /// Pointer to fcp- definition
  const G4ParticleDefinition* the_fcp_minus_;
  /// Pointer to fcp+ definition
  const G4ParticleDefinition* the_fcp_plus_;

  /// Temporary vector for element selection
  std::vector<G4double> temp_;

  /**
   * Enable logging for this class.
   */
  enableLogging("APrimeConversionToFCPs");
};

}  // namespace simcore

#endif  // SIMCORE_APRIMECONVERSIONTOFCPS_H_
