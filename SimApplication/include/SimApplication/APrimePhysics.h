#ifndef SIMAPPLICATION_APRIMEPHYSICS_H_
#define SIMAPPLICATION_APRIMEPHYSICS_H 1_

// Geant4
#include "G4VPhysicsConstructor.hh"
#include "G4Decay.hh"
#include "G4hMultipleScattering.hh"
#include "G4ProcessManager.hh"

namespace sim {

class APrimePhysics : public G4VPhysicsConstructor {

    public:

        APrimePhysics(const G4String& name = "APrime");

        virtual ~APrimePhysics();

        void ConstructParticle();

        void ConstructProcess();

    private:

        G4ParticleDefinition* aprimeDef;
        //G4Decay decayProcess;
        //G4hMultipleScattering scatterProcess;
};

}

#endif
