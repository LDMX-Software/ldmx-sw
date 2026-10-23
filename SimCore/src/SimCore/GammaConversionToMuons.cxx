#include "SimCore/GammaConversionToMuons.h"

#include <algorithm>
#include <cmath>

#include "G4DynamicParticle.hh"
#include "G4EmParameters.hh"
#include "G4Exp.hh"
#include "G4Log.hh"
#include "G4Material.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4NistManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4Track.hh"
#include "Randomize.hh"

namespace simcore {

G4VParticleChange* GammaConversionToMuons::PostStepDoIt(const G4Track& track,
                                                        const G4Step& step) {
  const G4double mmuon{G4MuonPlus::MuonPlus()->GetPDGMass()};
  const G4DynamicParticle* gamma{track.GetDynamicParticle()};
  const G4double egam{gamma->GetKineticEnergy()};

  // Geant4's own sampling near threshold and in the 5D model range
  if (egam <= 5. * mmuon ||
      egam <= G4EmParameters::Instance()->MaxEnergyFor5DMuPair()) {
    return G4GammaConversionToMuons::PostStepDoIt(track, step);
  }

  aParticleChange.Initialize(track);
  aParticleChange.ProposeMomentumDirection(0., 0., 0.);
  aParticleChange.ProposeEnergy(0.);
  aParticleChange.ProposeTrackStatus(fStopAndKill);

  const G4Element* element{selectElement(egam, track.GetMaterial())};
  const G4double a027{G4NistManager::Instance()->GetA27(element->GetZasInt())};

  G4double x_plus{0.5}, theta_plus{0.}, theta_minus{0.}, phi_half{0.};
  sampleTsai(egam, a027, x_plus, theta_plus, theta_minus, phi_half);
  const G4double x_minus{1. - x_plus};

  // Directions for a photon along z, azimuth uniform
  const G4double phi0{CLHEP::twopi * G4UniformRand()};
  G4ThreeVector dir_plus(std::sin(theta_plus) * std::cos(phi0 + phi_half),
                         std::sin(theta_plus) * std::sin(phi0 + phi_half),
                         std::cos(theta_plus));
  G4ThreeVector dir_minus(-std::sin(theta_minus) * std::cos(phi0 - phi_half),
                          -std::sin(theta_minus) * std::sin(phi0 - phi_half),
                          std::cos(theta_minus));
  dir_plus.rotateUz(gamma->GetMomentumDirection());
  dir_minus.rotateUz(gamma->GetMomentumDirection());

  aParticleChange.AddSecondary(new G4DynamicParticle(
      G4MuonPlus::MuonPlus(), dir_plus, x_plus * egam - mmuon));
  aParticleChange.AddSecondary(new G4DynamicParticle(
      G4MuonMinus::MuonMinus(), dir_minus, x_minus * egam - mmuon));
  return G4VDiscreteProcess::PostStepDoIt(track, step);
}

const G4Element* GammaConversionToMuons::selectElement(
    G4double egam, const G4Material* material) {
  const G4ElementVector* elements{material->GetElementVector()};
  const G4double* atoms_per_volume{material->GetVecNbOfAtomsPerVolume()};
  const std::size_t n{material->GetNumberOfElements()};
  if (n == 1) return (*elements)[0];

  std::vector<G4double> cumulative(n, 0.);
  G4double sum{0.};
  for (std::size_t i = 0; i < n; ++i) {
    sum += atoms_per_volume[i] *
           ComputeCrossSectionPerAtom(egam, (*elements)[i]->GetZasInt());
    cumulative[i] = sum;
  }
  const G4double r{sum * G4UniformRand()};
  for (std::size_t i = 0; i < n; ++i) {
    if (r <= cumulative[i]) return (*elements)[i];
  }
  return (*elements)[n - 1];
}

void GammaConversionToMuons::sampleTsai(G4double egam, G4double a027,
                                        G4double& x_plus, G4double& theta_plus,
                                        G4double& theta_minus,
                                        G4double& phi_half) const {
  // All kinematics in units of the muon mass
  const G4double mmuon{G4MuonPlus::MuonPlus()->GetPDGMass()};
  const G4double gamma_muon_inv{mmuon / egam};
  const G4double gamma0{1. / gamma_muon_inv};
  const G4double sqrtx{std::sqrt(0.25 - gamma_muon_inv)};
  const G4double xmin{0.5 - sqrtx};
  const G4double x_range{2. * sqrtx};
  const G4int nmax{1000};

  // Weights reach ~1.3 at TeV, below 1 at 1-10 GeV
  const G4double max_weight{1.5};

  // q/mMu scale of the form factor, PRM (6.115)
  const G4double q_form_factor{1. / (0.20 * a027)};

  // log(rho) range scale, only flattens the max weight in energy
  const G4double x4{std::pow(2. * gamma_muon_inv, 4.)};
  const G4double a_range_norm{G4Log((std::pow(q_form_factor, 4.) + x4) / x4)};

  G4bool good_event{false};
  G4int nn{0};
  do {
    ++nn;

    const G4double xp{xmin + G4UniformRand() * x_range};
    const G4double xm{1. - xp};
    const G4double del_over_mmuon{mmuon / (2. * egam * xp * xm)};

    // Sample t, psi and a = log(rho^4+c2^4)
    const G4double t{G4UniformRand()};
    const G4double u{std::sqrt(1. / t - 1.)};
    const G4double psi{CLHEP::twopi * G4UniformRand()};

    const G4double rho_max{std::min(2. * u / std::abs(std::cos(psi)),
                                    CLHEP::pi * u / std::abs(std::sin(psi)))};
    const G4double c24{std::pow(del_over_mmuon / t, 4.)};
    const G4double amin{G4Log(c24)};
    const G4double a_range{G4Log(std::pow(rho_max, 4.) + c24) - amin};
    const G4double a{amin + G4UniformRand() * a_range};
    const G4double rho{std::pow(G4Exp(a) - c24, 0.25)};

    const G4double xi_half{0.5 * rho * std::cos(psi)};
    const G4double phi{rho / u * std::sin(psi)};
    const G4double u_plus{u + xi_half};
    const G4double u_minus{u - xi_half};

    // Reject angles beyond pi
    if (u_plus > xp * CLHEP::pi / gamma_muon_inv ||
        u_minus > xm * CLHEP::pi / gamma_muon_inv) {
      continue;
    }

    const G4double th_p{gamma_muon_inv * u_plus / xp};
    const G4double gamma_p{gamma0 * xp};
    const G4double beta_p{std::sqrt(1. - 1. / (gamma_p * gamma_p))};
    const G4double kdotp_p{gamma0 * gamma_p * (1. - beta_p * std::cos(th_p))};
    const G4double pperp_p{beta_p * gamma_p * std::sin(th_p)};

    const G4double th_m{gamma_muon_inv * u_minus / xm};
    const G4double gamma_m{gamma0 * xm};
    const G4double beta_m{std::sqrt(1. - 1. / (gamma_m * gamma_m))};
    const G4double kdotp_m{gamma0 * gamma_m * (1. - beta_m * std::cos(th_m))};
    const G4double pperp_m{beta_m * gamma_m * std::sin(th_m)};

    // Positive squared momentum transfer
    const G4double qpar{gamma0 - beta_m * gamma_m * std::cos(th_m) -
                        beta_p * gamma_p * std::cos(th_p)};
    const G4double q2perp{(pperp_p - pperp_m) * (pperp_p - pperp_m) +
                          2. * pperp_p * pperp_m * (1. - std::cos(phi))};
    const G4double q2{qpar * qpar + q2perp};

    // Measure and Jacobians from (a, t, psi) to (u+, u-, phi)
    const G4double measure{x_range * a_range / a_range_norm};
    const G4double jacobian{G4Exp(a) / (rho * rho * rho) * rho /
                            (t * (1. - t))};

    // Tsai 1974 matrix element
    const G4double term1_p{-(-q2 + kdotp_m) / kdotp_p};
    const G4double term1_m{-(-q2 + kdotp_p) / kdotp_m};
    const G4double term2_p{(-q2 / 2. + 2. * gamma_m * gamma_m) /
                           (kdotp_p * kdotp_p)};
    const G4double term2_m{(-q2 / 2. + 2. * gamma_p * gamma_p) /
                           (kdotp_m * kdotp_m)};
    const G4double mixed{2. *
                         ((1. + q2 / 2.) * (-2. * gamma_m * gamma_p - q2 / 2.) +
                          q2 * gamma0 * gamma0 / 2.) /
                         (kdotp_m * kdotp_p)};
    const G4double matrix_el{std::sin(th_p) * std::sin(th_m) / (4. * q2 * q2) *
                             (term1_p + term1_m + term2_p + term2_m + mixed)};

    const G4double ff_sqrt{1. / (1. + q2 / (q_form_factor * q_form_factor))};
    const G4double weight{measure * jacobian * matrix_el * ff_sqrt * ff_sqrt};

    if (weight < 0. || weight > max_weight) {
      G4cerr << "GammaConversionToMuons: weight " << weight
             << " out of bounds 0 to " << max_weight << G4endl;
    }

    x_plus = xp;
    theta_plus = th_p;
    theta_minus = th_m;
    phi_half = 0.5 * phi;

    if (G4UniformRand() * max_weight < weight) good_event = true;
  } while (nn < nmax && !good_event);

  if (!good_event) {
    G4cerr << "GammaConversionToMuons: no event accepted after " << nmax
           << " tries, using the last sampled kinematics" << G4endl;
  }
}

}  // namespace simcore
