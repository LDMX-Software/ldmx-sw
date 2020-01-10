/**
 * @file APrimePhysics.cxx
 * @brief Class which defines basic APrime physics
 * @author Michael Revering, University of Minnesota
 */

#include "SimApplication/APrimePhysics.h"

// LDMX
#include "Exception/Exception.h"
#include "SimCore/G4APrime.h"
#include "SimCore/G4eDarkBremsstrahlung.h"

// Geant4
#include "G4ProcessManager.hh"
#include "G4Electron.hh"

namespace ldmx {

    APrimePhysics::APrimePhysics(const G4String& name) :
            G4VPhysicsConstructor(name), aprimeDef_(nullptr) { }

    APrimePhysics::~APrimePhysics() { }

    void APrimePhysics::ConstructParticle() { 
        /**
         * Insert A-prime into the Geant4 particle table.
         * For now we flag it as stable.
         */
        aprimeDef_ = G4APrime::APrime();

        //aprimeDef->SetProcessManager(new G4ProcessManager(aprimeDef));
    }

    void APrimePhysics::ConstructProcess() {
        /*
         G4ProcessManager* pm = aprimeDef->GetProcessManager();
         if (pm != NULL) {
         pm->AddProcess(&scatterProcess, -1, 1, 1);
         pm->AddProcess(&decayProcess, -1, -1, 2);
         } else {
            EXCEPTION_RAISE( "InitializationError",
                "The process manager for APrime is NULL.");
         }
         */
    	G4ParticleDefinition* particle = G4Electron::ElectronDefinition();
    	G4ProcessManager* pm = particle->GetProcessManager();
    	pm->AddProcess(new G4eDarkBremsstrahlung(), -1, 1, 1);
    }

}
