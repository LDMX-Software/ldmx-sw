#include "SimApplication/APrimePhysics.h"

// Geant4
#include "G4SystemOfUnits.hh"

namespace sim {

APrimePhysics::APrimePhysics(const G4String& name)
    : G4VPhysicsConstructor(name), aprimeDef(NULL) {
}

APrimePhysics::~APrimePhysics() {
}

void APrimePhysics::ConstructParticle() {

    /**
     * Insert A-prime into the Geant4 particle table.
     * For now we flag it as stable.
     */
    aprimeDef = new G4ParticleDefinition(
            "A^1", /* name */
            0.003 * GeV, /* mass */
            0, /* width */
            0, /* charge */
            0, /* 2*spin */
            0, /* parity */
            0, /* C-conjugation */
            0, /* 2*isospin */
            0, /* 2*isospin3 */
            0, /* G-parity */
            "APrime", /* type */
            0, /* lepton number */
            0, /* baryon number */
            622, /* PDG encoding */
            true, /* stable */
            0, /*DBL_MIN,*/ /* lifetime (may be overridden by predefined decay time) */
            0, /* decay table */
            false /* short lived */
    );

    //aprimeDef->SetProcessManager(new G4ProcessManager(aprimeDef));
}

void APrimePhysics::ConstructProcess() {
    /*
    G4ProcessManager* pm = aprimeDef->GetProcessManager();
    if (pm != NULL) {
        pm->AddProcess(&scatterProcess, -1, 1, 1);
        pm->AddProcess(&decayProcess, -1, -1, 2);
    } else {
        G4Exception("APrimePhysics::ConstructProcess",
                "InitializationError",
                FatalException,
                "The process manager for APrime is NULL.");
    }
    */
}

}
