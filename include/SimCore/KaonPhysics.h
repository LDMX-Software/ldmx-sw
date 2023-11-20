#ifndef SIMCORE_KAON_PHYSICS_H
#define SIMCORE_KAON_PHYSICS_H

#include <G4DecayTable.hh>
#include <G4KaonMinus.hh>
#include <G4KaonPlus.hh>
#include <G4KaonZeroLong.hh>
#include <G4KaonZeroShort.hh>
#include <G4ParticleDefinition.hh>
#include <G4VDecayChannel.hh>
#include <G4VPhysicsConstructor.hh>
#include <numeric>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

namespace simcore {

/**
 * @class KaonPhysics
 *
 * @brief Allows the configuration of properties of kaons
 * produced in the simulation, in particular setting their lifetime and
 * branching ratios
 *
 */

class KaonPhysics : public G4VPhysicsConstructor {
 private:
  /**
   *
   * Represents the 6 possible decay channels for charged kaons in Geant4 (See
   * G4KaonMinus.cc and G4KaonPlus.cc in the Geant4 sources) in the order they
   * appear in the decay table.
   *
   * The processes are
   * K^+ -> \mu^+ + \nu_\mu
   * K^+ -> \pi^+ + \pi^0
   * K^+ -> \pi^+ + \pi^- + \pi^+
   * K^+ -> \pi^+ + \pi^0 + \pi^0
   * K^+ -> \pi^0 + e^+ + \nu_e
   * K^+ -> \pi^0 + \mu^+ + \nu_\mu
   *
   * And vice versa for K^-.
   * The indices here correspond to the position of the branching ratio for
   * that process in the corresponding parameter as well as the position in
   * the decay table.
   */
  enum ChargedKaonDecayChannel {
    mu_nu = 0,
    pi_pi0 = 1,
    pi_pi_pi = 2,
    pi_pi0_pi0 = 3,
    pi0_e_nu = 4,
    pi0_mu_nu = 5
  };
  /*
   *
   * Corresponding entries for the K^0_L. Note that K^0_L and K^0_S decays are
   * not symmetric like the charged ones so they need to be handled manually.
   *
   * The processes are
   *
   * K^0_L -> \pi^0 + \pi^0 + \pi^0
   * K^0_L -> \pi^0 + \pi^+ + \pi^-
   * K^0_L -> \pi^- + e^+ + \nu_e
   * K^0_L -> \pi^+ + e^- + \nu_e
   * K^0_L -> \pi^- + \mu^+ + \nu_\mu
   * K^0_L -> \pi^+ + \mu^- + \nu_\mu
   *
   * and
   *
   * K^0_S -> \pi^+ + \pi^-
   * K^0_S -> \pi^0 + \pi^0
   *
   **/
  enum KaonZeroLongDecayChannel {
    pi0_pi0_pi0 = 0,
    pi0_pip_pim = 1,
    pip_e_nu = 2,
    pim_e_nu = 3,
    pim_mu_nu = 4,
    pip_mu_nu = 5,
  };
  enum KaonZeroShortDecayChannel {
    pip_pim = 0,
    pi0_pi0 = 1,
  };

  // Factor to scale the K^+/K^-/K^0_L/K^0_S lifetimes by. To reduce the
  // lifetime of all charged kaons by a factor 50, set these to 1/50.
  double kplus_lifetime_factor{1};
  double kminus_lifetime_factor{1};
  double k0l_lifetime_factor{1};
  double k0s_lifetime_factor{1};

  // Branching ratios for each of the decay processes listed in the
  // KaonDecayChannel enumerators
  std::vector<double> kplus_branching_ratios;
  std::vector<double> kminus_branching_ratios;
  std::vector<double> k0l_branching_ratios;
  std::vector<double> k0s_branching_ratios;

 public:
  KaonPhysics(const G4String& name,
              const framework::config::Parameters& parameters);
  virtual ~KaonPhysics() = default;

  /**
   *  Set the lifetime and branching ratios for one of the kaon species.
   */
  void setDecayProperties(G4ParticleDefinition* kaon,
                          const std::vector<double>& branching_ratios,
                          double lifetime_factor) const;

  /**
   * Construct/Update particles
   *
   * Update the particle definitions for charged kaons
   *
   * 1. Scale their lifetime by the corresponding lifetime factor
   *
   * 2. Set their branching ratios to those in the corresponding branching
   * raito parameter
   */

  void ConstructParticle() override;

  /**
   * Construct processes
   *
   * We don't do anything here since we are just attaching/updating
   * the kaon particle definitions.
   */
  void ConstructProcess() override{};
};
}  // namespace simcore

#endif /* SIMCORE_KAON_PHYSICS_H */
