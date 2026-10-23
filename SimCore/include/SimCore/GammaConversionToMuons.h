/**
 * @file GammaConversionToMuons.h
 * @brief gamma -> mu+ mu- with the full Tsai matrix element final state
 *
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_GAMMACONVERSIONTOMUONS_H_
#define SIMCORE_GAMMACONVERSIONTOMUONS_H_

#include "G4GammaConversionToMuons.hh"

namespace simcore {

/**
 * @class GammaConversionToMuons
 * @brief Geant4's gamma -> mu+ mu- process with a Tsai (1974) final state
 *
 * Cross sections, the 5D model and the sampling near threshold are Geant4's.
 * Above 5 muon masses the pair kinematics are sampled from the full Tsai
 * matrix element with a dipole form factor (exact up to 1/M_nucleus and
 * inelastic terms) instead of Geant4's factorized approximation. This was
 * an LDMX patch to Geant4 10.2.3, written by N. Toro (muonNotes in
 * LDMX-Software/geant4 LDMX.10.2.3_v0.6).
 *
 * The process name stays "GammaToMuPair" so biasing operators find it.
 */
class GammaConversionToMuons : public G4GammaConversionToMuons {
 public:
  GammaConversionToMuons() = default;
  ~GammaConversionToMuons() override = default;

  /**
   * Generate the final state, Tsai sampling above 5 muon masses
   */
  G4VParticleChange* PostStepDoIt(const G4Track& track,
                                  const G4Step& step) override;

 private:
  /**
   * Pick an element of the material weighted by its cross section
   */
  const G4Element* selectElement(G4double egam, const G4Material* material);

  /**
   * Sample the pair kinematics from the Tsai matrix element
   *
   * @param[in] egam photon energy
   * @param[in] a027 A^0.27 of the target nucleus
   * @param[out] x_plus energy fraction of the mu+
   * @param[out] theta_plus mu+ polar angle
   * @param[out] theta_minus mu- polar angle
   * @param[out] phi_half half of the azimuthal opening angle
   */
  void sampleTsai(G4double egam, G4double a027, G4double& x_plus,
                  G4double& theta_plus, G4double& theta_minus,
                  G4double& phi_half) const;
};

}  // namespace simcore

#endif  // SIMCORE_GAMMACONVERSIONTOMUONS_H_
