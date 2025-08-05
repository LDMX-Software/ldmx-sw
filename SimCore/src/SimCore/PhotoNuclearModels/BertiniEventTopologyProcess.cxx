#include "SimCore/PhotoNuclearModels/BertiniEventTopologyProcess.h"
namespace simcore {

void BertiniEventTopologyProcess::cleanupSecondaries() {
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  // Geant4 won't clean up this memory for us by default
  for (int i{0}; i < secondaries; ++i) {
    // disable clang's use-after-free warning for this delete
    // as far as we (@tomeichlersmith and @EinarElen) can tell,
    // this is correct and clang is just getting confused since
    // Geant4's inlined delete method is written in a wacky way
#ifndef __clang_analyzer__
    auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    delete secondary;
#endif
  }
}

G4HadFinalState* BertiniEventTopologyProcess::ApplyYourself(
    const G4HadProjectile& projectile, G4Nucleus& targetNucleus) {
  int attempts{1};
  if (!acceptProjectile(projectile) || !acceptTarget(targetNucleus)) {
    // Bertini will handle the particle change on its own here
    return G4CascadeInterface::ApplyYourself(projectile, targetNucleus);
  }

  while (true) {
    theParticleChange.clear();
    theParticleChange.SetStatusChange(stopAndKill);
    G4CascadeInterface::ApplyYourself(projectile, targetNucleus);
    if (acceptEvent()) {
      incrementEventWeight(attempts);
      return &theParticleChange;
    }
    attempts++;
    cleanupSecondaries();
  }
}

}  // namespace simcore
