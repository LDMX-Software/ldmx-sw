#ifndef SIMAPPLICATION_GAMMAPHYSICS_H_
#define SIMAPPLICATION_GAMMAPHYSICS_H 1_

// Geant4
#include "G4VPhysicsConstructor.hh"
#include "G4GammaConversionToMuons.hh"

namespace sim {

class GammaPhysics : public G4VPhysicsConstructor {

    public:

        GammaPhysics(const G4String& name = "GammaPhysics");

        virtual ~GammaPhysics();

        void ConstructParticle() {}

        void ConstructProcess();

    private:

        G4GammaConversionToMuons gammaConvProcess;
};

}

#endif
