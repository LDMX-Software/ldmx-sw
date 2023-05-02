#ifndef SIMCORE_BERTINI_EVENTTOPOLOGY_PROCESS_H
#define SIMCORE_BERTINI_EVENTTOPOLOGY_PROCESS_H

#include <iostream>

#include "G4CascadeInterface.hh"
#include "G4EventManager.hh"
#include "G4HadFinalState.hh"
#include "G4HadProjectile.hh"
#include "G4HadronicInteraction.hh"
#include "G4Nucleus.hh"
#include "SimCore/PhotonuclearModel.h"
#include "SimCore/UserEventInformation.h"
namespace simcore {
class BertiniEventTopologyProcess : public G4CascadeInterface {
 public:
  BertiniEventTopologyProcess() : G4CascadeInterface{} {}

  /*
   * The primary function for derived classes to customize. After each call to
   * the Bertini cascade, this function will be called to see whether or not to
   * keep the event. The products can be accessed from the `theParticleChange`
   * member inherited from G4CascadeInterface (i.e. the Bertini cascade).
   */
  virtual bool acceptEvent() const = 0;

  /*
   * Is the projectile of interest?
   *
   * If false, the cascade will only run once. Example use includes only
   * applying repeated simulations for particular energy ranges. Is called
   * automatically during `ApplyYourself`.
   *
   **/
  virtual bool acceptProjectile(const G4HadProjectile& projectile) const = 0;

  /*
   * Is the target nucleus of interest?
   *
   * If false, the cascade will only run once. Example use includes only
   * applying repeated simulations for high Z materials. Is called
   * automatically during `ApplyYourself`.
   *
   **/
  virtual bool acceptTarget(const G4Nucleus& targetNucleus) const = 0;

  /*
   * Run the Bertini cascade until the condition given by `acceptEvent` is
   * matched by the reaction products. Increments the event weight by 1/n where
   * n is the number of attempts needed to produce a match.
   *
   **/
  G4HadFinalState* ApplyYourself(const G4HadProjectile& projectile,
                                 G4Nucleus& targetNucleus) override;

  /*
   * Geant4 assumes that secondaries produced from the bertini cascade are owned
   *  by some other part of the code. Since we are re-running the cascade until
   * we get something matching our condition in `acceptEvent`, we have to make
   * sure to clean up any secondaries from failed attempts.
   *
   */
  void cleanupSecondaries();

  /**
   *  Check if the PDG code corresponds to a light ion nucleus.
   *
   *  Nuclear PDG codes are given by ±10LZZZAAAI So to find the atomic
   *  number, we first divide by 10 (to lose the I-component) and then
   *  take the modulo with 1000.
   *
   */
  constexpr bool isLightNucleus(const int pdgCode) const {
    //
    // TODO: Is the < check necessary?
    if (pdgCode > 1000000000 && pdgCode < 10000000000) {
      // Check if the atomic number is less than or equal to 4
      return ((pdgCode / 10) % 1000) <= 4;
    }
    return false;
  }
};
}  // namespace simcore

#endif /* SIMCORE_BERTINI_EVENTTOPOLOGY_PROCESS_H */
