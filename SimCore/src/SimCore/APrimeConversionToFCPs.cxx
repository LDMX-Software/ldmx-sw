#include "SimCore/APrimeConversionToFCPs.h"

#include "G4DarkBreM/G4APrime.h"
#include "G4DarkBreM/G4FractionallyCharged.h"
#include "G4EventManager.hh"
#include "G4Exp.hh"
#include "G4Log.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "Randomize.hh"
#include "SimCore/G4User/UserEventInformation.h"

namespace simcore {

const std::string APrimeConversionToFCPs::PROCESS_NAME = "APrimeToFCPPair";

static const G4double SQRTE = std::sqrt(std::exp(1.));
static const G4double POW_SAT = -0.88;

APrimeConversionToFCPs::APrimeConversionToFCPs(const G4String& processName,
                                               G4ProcessType type)
    : G4VDiscreteProcess(processName, type) {
  // Get the fcp particle definitions
  auto* fcp_minus = G4FractionallyCharged::FractionallyCharged();
  mfcp_ = fcp_minus->GetPDGMass();

  // Get fcp charge magnitude in units of e
  G4double qfcp = std::abs(fcp_minus->GetPDGCharge() / CLHEP::eplus);

  // Coupling-scaled reduced Compton wavelength
  // rc_ = q^2 * elm_coupling / mfcp_
  // This gives the correct q^4 scaling in sigfac = 4*alpha*Z^2*rc_^2
  rc_ = qfcp * qfcp * CLHEP::elm_coupling / mfcp_;

  limit_energy_ = 5. * mfcp_;
  lowest_energy_limit_ = 2. * mfcp_;
  highest_energy_limit_ = 1e12 * CLHEP::GeV;
  ldmx_log(debug) << "Energy limits: lowest="
                  << lowest_energy_limit_ / CLHEP::MeV
                  << " MeV, limit=" << limit_energy_ / CLHEP::MeV
                  << " MeV, highest=" << highest_energy_limit_ / CLHEP::GeV
                  << " GeV";

  the_a_prime_ = G4APrime::APrime();
  the_fcp_minus_ = fcp_minus;
  // Look up the antiparticle (fcp+) from the particle table
  the_fcp_plus_ = G4ParticleTable::GetParticleTable()->FindParticle("fcp+");

  ldmx_log(debug) << "A' particle: "
                  << (the_a_prime_ ? the_a_prime_->GetParticleName() : "NULL")
                  << ", mass="
                  << (the_a_prime_ ? the_a_prime_->GetPDGMass() / CLHEP::MeV
                                   : 0)
                  << " MeV";
  ldmx_log(debug) << "FCP+ particle: "
                  << (the_fcp_plus_ ? the_fcp_plus_->GetParticleName()
                                    : "NULL");
  ldmx_log(debug) << "FCP- particle: "
                  << (the_fcp_minus_ ? the_fcp_minus_->GetParticleName()
                                     : "NULL");
}

G4bool APrimeConversionToFCPs::IsApplicable(
    const G4ParticleDefinition& particle) {
  bool applicable = (&particle == the_a_prime_);
  ldmx_log(debug) << "IsApplicable called for particle: "
                  << particle.GetParticleName() << " -> "
                  << (applicable ? "YES (this is A')" : "NO");
  return applicable;
}

void APrimeConversionToFCPs::BuildPhysicsTable(const G4ParticleDefinition& p) {
  ldmx_log(debug) << "=== BuildPhysicsTable for particle: "
                  << p.GetParticleName() << " ===";
  auto table = G4Material::GetMaterialTable();
  std::size_t nelm = 0;
  for (auto const& mat : *table) {
    std::size_t n = mat->GetNumberOfElements();
    nelm = std::max(nelm, n);
    ldmx_log(debug) << "  Material: " << mat->GetName() << " with " << n
                    << " elements";
  }
  temp_.resize(nelm, 0);
  ldmx_log(debug) << "Max elements in any material: " << nelm;
  printInfoDefinition();
}

G4double APrimeConversionToFCPs::GetMeanFreePath(const G4Track& aTrack,
                                                 G4double, G4ForceCondition*) {
  const G4DynamicParticle* a_dynamic_a_prime = aTrack.GetDynamicParticle();
  G4double a_prime_energy = a_dynamic_a_prime->GetKineticEnergy();
  const G4Material* a_material = aTrack.GetMaterial();
  G4double mfp = computeMeanFreePath(a_prime_energy, a_material);
  ldmx_log(trace) << "GetMeanFreePath: A' KE=" << a_prime_energy / CLHEP::MeV
                  << " MeV in " << a_material->GetName()
                  << " -> MFP=" << (mfp < DBL_MAX ? mfp / CLHEP::mm : -1)
                  << " mm";
  return mfp;
}

G4double APrimeConversionToFCPs::computeMeanFreePath(
    G4double APrimeEnergy, const G4Material* aMaterial) {
  if (APrimeEnergy <= lowest_energy_limit_) {
    return DBL_MAX;
  }

  const G4ElementVector* the_element_vector = aMaterial->GetElementVector();
  const G4double* nb_of_atoms_per_volume =
      aMaterial->GetVecNbOfAtomsPerVolume();

  G4double sigma = 0.0;
  G4double fact = 1.0;
  G4double e = APrimeEnergy;

  // Low energy approximation as in Bethe-Heitler model
  if (e < limit_energy_) {
    G4double y =
        (e - lowest_energy_limit_) / (limit_energy_ - lowest_energy_limit_);
    fact = y * y;
    e = limit_energy_;
  }

  for (std::size_t i = 0; i < aMaterial->GetNumberOfElements(); ++i) {
    sigma += nb_of_atoms_per_volume[i] * fact *
             computeCrossSectionPerAtom(
                 e, G4lrint((*the_element_vector)[i]->GetZ()));
  }
  return (sigma > 0.0) ? 1. / sigma : DBL_MAX;
}

G4double APrimeConversionToFCPs::getCrossSectionPerAtom(
    const G4DynamicParticle* aDynamicAPrime, const G4Element* anElement) {
  return computeCrossSectionPerAtom(aDynamicAPrime->GetKineticEnergy(),
                                    G4lrint(anElement->GetZ()));
}

G4double APrimeConversionToFCPs::computeCrossSectionPerAtom(G4double Egam,
                                                            G4int Z) {
  // Cross section parametrization adapted from H. Burkhardt
  // (G4GammaConversionToMuons) with Mmuon -> mfcp_ and
  // coupling scaled by q_fcp^4 through rc_
  if (Egam <= lowest_energy_limit_) {
    return 0.0;
  }

  G4NistManager* nist = G4NistManager::Instance();

  G4double pow_thres, ecor, b, dn, zthird, winfty, w_med_appr, wsatur, sigfac;

  if (Z == 1) {  // special case of Hydrogen
    b = 202.4;
    dn = 1.49;
  } else {
    b = 183.;
    dn = 1.54 * nist->GetA27(Z);
  }
  zthird = 1. / nist->GetZ13(Z);  // Z^(-1/3)
  winfty = b * zthird * mfcp_ / (dn * electron_mass_c2);
  w_med_appr = 1. / (4. * dn * SQRTE * mfcp_);
  wsatur = winfty / w_med_appr;
  sigfac = 4. * fine_structure_const * Z * Z * rc_ * rc_;
  pow_thres = 1.479 + 0.00799 * dn;
  ecor = -18. + 4347. / (b * zthird);

  G4double cor_fuc = 1. + .04 * G4Log(1. + ecor / Egam);
  G4double eg = G4Exp(G4Log(1. - 4. * mfcp_ / Egam) * pow_thres) *
                G4Exp(G4Log(G4Exp(G4Log(wsatur) * POW_SAT) +
                            G4Exp(G4Log(Egam) * POW_SAT)) /
                      POW_SAT);

  G4double cross_section =
      7. / 9. * sigfac * G4Log(1. + w_med_appr * cor_fuc * eg);
  cross_section *= cross_sec_factor_;
  return cross_section;
}

void APrimeConversionToFCPs::setCrossSecFactor(G4double fac) {
  if (fac < 0.0) return;
  cross_sec_factor_ = fac;
  ldmx_log(info) << "APrimeConversionToFCPs cross section factor set to "
                 << cross_sec_factor_;
}

G4VParticleChange* APrimeConversionToFCPs::PostStepDoIt(const G4Track& aTrack,
                                                        const G4Step& aStep) {
  ldmx_log(debug) << "A' -> fcp+ fcp- conversion starting!";
  aParticleChange.Initialize(aTrack);
  const G4Material* a_material = aTrack.GetMaterial();

  ldmx_log(debug) << "Conversion in material: " << a_material->GetName();
  ldmx_log(debug) << "A' Track ID: " << aTrack.GetTrackID()
                  << ", Parent ID: " << aTrack.GetParentID();

  // Current A' energy and direction
  const G4DynamicParticle* a_dynamic_a_prime = aTrack.GetDynamicParticle();
  G4double egam = a_dynamic_a_prime->GetKineticEnergy();

  G4ThreeVector pos = aTrack.GetPosition();
  ldmx_log(debug) << "A' kinetic energy: " << egam / CLHEP::MeV << " MeV";
  ldmx_log(debug) << "A' position: (" << pos.x() / CLHEP::mm << ", "
                  << pos.y() / CLHEP::mm << ", " << pos.z() / CLHEP::mm
                  << ") mm";
  ldmx_log(debug) << "A' momentum direction: "
                  << a_dynamic_a_prime->GetMomentumDirection();

  if (egam <= lowest_energy_limit_) {
    ldmx_log(warn) << "A' energy (" << egam / CLHEP::MeV
                   << " MeV) below threshold ("
                   << lowest_energy_limit_ / CLHEP::MeV
                   << " MeV), skipping conversion!";
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  // Kill the incident A'
  ldmx_log(debug) << "Killing incident A' and creating fcp+/fcp- pair...";
  aParticleChange.ProposeMomentumDirection(0., 0., 0.);
  aParticleChange.ProposeEnergy(0.);
  aParticleChange.ProposeTrackStatus(fStopAndKill);

  G4ParticleMomentum a_prime_direction =
      a_dynamic_a_prime->GetMomentumDirection();

  // Select randomly one element constituting the material
  const G4Element* an_element = selectRandomAtom(a_dynamic_a_prime, a_material);
  G4int z = G4lrint(an_element->GetZ());

  // Store the element Z where the A' conversion occurred for downstream DQM.
  auto* event_info = static_cast<UserEventInformation*>(
      G4EventManager::GetEventManager()->GetUserInformation());
  if (event_info != nullptr) {
    event_info->setAPrimeConversionMaterialZ(z);
  }

  G4NistManager* nist = G4NistManager::Instance();

  G4double b, dn;
  G4double a027 = nist->GetA27(z);

  if (z == 1) {
    b = 202.4;
    dn = 1.49;
  } else {
    b = 183.;
    dn = 1.54 * a027;
  }
  G4double zthird = 1. / nist->GetZ13(z);
  G4double winfty = b * zthird * mfcp_ / (dn * electron_mass_c2);

  G4double c1_num = 0.138 * a027;
  G4double c1_num2 = c1_num * c1_num;
  G4double c2_term2 = electron_mass_c2 / (183. * zthird * mfcp_);

  G4double gamma_fcp_inv = mfcp_ / egam;

  // Generate xPlus according to the differential cross section by rejection
  G4double xmin =
      (egam <= limit_energy_) ? 0.5 : 0.5 - std::sqrt(0.25 - gamma_fcp_inv);
  G4double xmax = 1. - xmin;

  G4double ds2 = (dn * SQRTE - 2.);
  G4double s_bz = SQRTE * b * zthird / electron_mass_c2;
  G4double log_wmax_inv = 1. / G4Log(winfty * (1. + 2. * ds2 * gamma_fcp_inv) /
                                     (1. + 2. * s_bz * mfcp_ * gamma_fcp_inv));
  G4double x_plus = 0.5;
  G4double x_minus = 0.5;
  G4double x_pm = 0.25;

  G4int nn = 0;
  const G4int nmax = 1000;

  // Sampling for Egam > limit_energy_
  if (xmin < 0.5) {
    G4double result, w;
    do {
      x_plus = xmin + G4UniformRand() * (xmax - xmin);
      x_minus = 1. - x_plus;
      x_pm = x_plus * x_minus;
      G4double del = mfcp_ * mfcp_ / (2. * egam * x_pm);
      w = winfty * (1. + ds2 * del / mfcp_) / (1. + s_bz * del);
      G4double xxp = 1. - 4. / 3. * x_pm;
      result = (xxp > 0.) ? xxp * G4Log(w) * log_wmax_inv : 0.0;
      if (result > 1.0) {
        ldmx_log(warn) << "APrimeConversionToFCPs::PostStepDoIt WARNING:"
                       << " in dSigxPlusGen, result=" << result << " > 1";
      }
      ++nn;
      if (nn >= nmax) {
        break;
      }
    } while (G4UniformRand() > result);
  }

  // Generate angular variables via auxiliary variables t, psi, rho
  G4double t;
  G4double psi;
  G4double rho;

  G4double a3 = (gamma_fcp_inv / (2. * x_pm));
  G4double a33 = a3 * a3;
  G4double f1;
  G4double b1 = 1. / (4. * c1_num2);
  G4double b3 = b1 * b1 * b1;
  G4double a21 = a33 + b1;

  G4double f1_max =
      -(1. - x_pm) * (2. * b1 + (a21 + a33) * G4Log(a33 / a21)) / (2 * b3);

  G4double theta_plus, theta_minus, phi_half;
  nn = 0;

  // t, psi, rho generation (while angle < pi)
  do {
    // Generate t by rejection method
    do {
      ++nn;
      t = G4UniformRand();
      G4double a34 = a33 / (t * t);
      G4double a22 = a34 + b1;
      if (std::abs(b1) < 0.0001 * a34) {
        f1 = (1. - 2. * x_pm + 4. * x_pm * t * (1. - t)) /
             (12. * a34 * a34 * a34 * a34);
      } else {
        f1 = -(1. - 2. * x_pm + 4. * x_pm * t * (1. - t)) *
             (2. * b1 + (a22 + a34) * G4Log(a34 / a22)) / (2 * b3);
      }
      if (f1 < 0.0 || f1 > f1_max) {
        f1 = 0.0;
      }
      if (nn > nmax) {
        break;
      }
    } while (G4UniformRand() * f1_max > f1);

    // Generate psi by rejection method
    G4double f2_max = 1. - 2. * x_pm * (1. - 4. * t * (1. - t));
    G4double f2;
    do {
      ++nn;
      psi = CLHEP::twopi * G4UniformRand();
      f2 =
          1. - 2. * x_pm + 4. * x_pm * t * (1. - t) * (1. + std::cos(2. * psi));
      if (f2 < 0 || f2 > f2_max) {
        f2 = 0.0;
      }
      if (nn >= nmax) {
        break;
      }
    } while (G4UniformRand() * f2_max > f2);

    // Generate rho by direct transformation
    G4double c2_term1 = gamma_fcp_inv / (2. * x_pm * t);
    G4double c22 = c2_term1 * c2_term1 + c2_term2 * c2_term2;
    G4double c2 = 4. * c22 * c22 / std::sqrt(x_pm);
    G4double rhomax = (1. / t - 1.) * 1.9 / a027;
    G4double beta = G4Log((c2 + rhomax * rhomax * rhomax * rhomax) / c2);
    rho = G4Exp(G4Log(c2 * (G4Exp(beta * G4UniformRand()) - 1.)) * 0.25);

    // Get kinematical variables from t and psi
    G4double u = std::sqrt(1. / t - 1.);
    G4double xi_half = 0.5 * rho * std::cos(psi);
    phi_half = 0.5 * rho / u * std::sin(psi);

    theta_plus = gamma_fcp_inv * (u + xi_half) / x_plus;
    theta_minus = gamma_fcp_inv * (u - xi_half) / x_minus;

    // Protection against infinite loop
    if (nn > nmax) {
      if (std::abs(theta_plus) > CLHEP::pi) {
        theta_plus = 0.0;
      }
      if (std::abs(theta_minus) > CLHEP::pi) {
        theta_minus = 0.0;
      }
    }
  } while (std::abs(theta_plus) > CLHEP::pi ||
           std::abs(theta_minus) > CLHEP::pi);

  // Construct the momentum vectors
  // Azimuthal symmetry: take phi0 random between 0 and 2pi
  G4double phi0 = CLHEP::twopi * G4UniformRand();
  G4double e_plus = x_plus * egam;
  G4double e_minus = x_minus * egam;

  // fcp+ fcp- directions for A' in z-direction
  G4ThreeVector fcp_plus_direction(
      std::sin(theta_plus) * std::cos(phi0 + phi_half),
      std::sin(theta_plus) * std::sin(phi0 + phi_half), std::cos(theta_plus));
  G4ThreeVector fcp_minus_direction(
      -std::sin(theta_minus) * std::cos(phi0 - phi_half),
      -std::sin(theta_minus) * std::sin(phi0 - phi_half),
      std::cos(theta_minus));

  // Rotate to actual A' direction
  fcp_plus_direction.rotateUz(a_prime_direction);
  fcp_minus_direction.rotateUz(a_prime_direction);

  // Create fcp+ secondary
  auto a_particle1 =
      new G4DynamicParticle(the_fcp_plus_, fcp_plus_direction, e_plus - mfcp_);
  aParticleChange.AddSecondary(a_particle1);

  ldmx_log(debug) << "Created FCP+ with:";
  ldmx_log(debug) << "  - Kinetic energy: " << (e_plus - mfcp_) / CLHEP::MeV
                  << " MeV";
  ldmx_log(debug) << "  - Total energy: " << e_plus / CLHEP::MeV << " MeV";
  ldmx_log(debug) << "  - Direction: " << fcp_plus_direction;

  // Create fcp- secondary
  auto a_particle2 = new G4DynamicParticle(the_fcp_minus_, fcp_minus_direction,
                                           e_minus - mfcp_);
  aParticleChange.AddSecondary(a_particle2);

  ldmx_log(debug) << "Created FCP- with:";
  ldmx_log(debug) << "  - Kinetic energy: " << (e_minus - mfcp_) / CLHEP::MeV
                  << " MeV";
  ldmx_log(debug) << "  - Total energy: " << e_minus / CLHEP::MeV << " MeV";
  ldmx_log(debug) << "  - Direction: " << fcp_minus_direction;

  ldmx_log(debug) << "=== A' -> fcp+ fcp- CONVERSION COMPLETE ===";

  return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
}  // end of PostStepDoIt

const G4Element* APrimeConversionToFCPs::selectRandomAtom(
    const G4DynamicParticle* aDynamicAPrime, const G4Material* aMaterial) {
  const std::size_t number_of_elements = aMaterial->GetNumberOfElements();
  const G4ElementVector* the_element_vector = aMaterial->GetElementVector();
  const G4Element* elm = (*the_element_vector)[0];

  if (number_of_elements > 1) {
    G4double e = std::max(aDynamicAPrime->GetKineticEnergy(), limit_energy_);
    const G4double* natom = aMaterial->GetVecNbOfAtomsPerVolume();

    G4double sum = 0.;
    for (std::size_t i = 0; i < number_of_elements; ++i) {
      elm = (*the_element_vector)[i];
      sum += natom[i] * computeCrossSectionPerAtom(e, G4lrint(elm->GetZ()));
      temp_[i] = sum;
    }
    sum *= G4UniformRand();
    for (std::size_t i = 0; i < number_of_elements; ++i) {
      if (sum <= temp_[i]) {
        elm = (*the_element_vector)[i];
        break;
      }
    }
  }
  return elm;
}

void APrimeConversionToFCPs::printInfoDefinition() {
  G4cout << G4endl << GetProcessName() << ":  "
         << "A' -> fcp+ fcp- Bethe-Heitler-like process" << G4endl;
  G4cout << "        good cross section parametrization from "
         << G4BestUnit(lowest_energy_limit_, "Energy") << " to "
         << highest_energy_limit_ / GeV << " GeV for all Z." << G4endl;
  G4cout << "        cross section factor: " << cross_sec_factor_ << G4endl;
}

}  // namespace simcore
