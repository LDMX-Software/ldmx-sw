#include "SimApplication/GammaPhysics.h"

// Geant4
#include "G4ProcessManager.hh"

namespace sim {

GammaPhysics::GammaPhysics(const G4String& name)
    : G4VPhysicsConstructor(name) {
}

GammaPhysics::~GammaPhysics() {
}

void GammaPhysics::ConstructProcess() {
    aParticleIterator->reset();
    while((*aParticleIterator)()) {
        G4ParticleDefinition* particle = aParticleIterator->value();
        G4ProcessManager* pmanager = particle->GetProcessManager();
        G4String particleName = particle->GetParticleName();
        if (particleName == "gamma") {
            pmanager->AddDiscreteProcess(&gammaConvProcess);
        }
    }
}

}
