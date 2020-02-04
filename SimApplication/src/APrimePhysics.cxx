/**
 * @file APrimePhysics.cxx
 * @brief Class which defines basic APrime physics
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimApplication/APrimePhysics.h"

// LDMX
#include "SimCore/G4APrime.h"

// Geant4
#include "G4Electron.hh"
#include "G4ProcessManager.hh"

namespace ldmx {

    APrimePhysics::APrimePhysics(const G4String& name) :
            G4VPhysicsConstructor(name), aprimeDef_(nullptr) { }

    APrimePhysics::~APrimePhysics() {
    }

    void APrimePhysics::ConstructParticle() { 
        /**
         * Insert A-prime into the Geant4 particle table.
         * For now we flag it as stable.
         *
         * Geant4 registers all instances derived from G4ParticleDefinition and 
         * deletes them at the end of the run.
         */
        aprimeDef_ = G4APrime::APrime(aprimeMass_);
    }

    void APrimePhysics::ConstructProcess() {

        //add process to electron
        G4eDarkBremsstrahlung *theDarkBremProcess = new G4eDarkBremsstrahlung;
        theDarkBremProcess->SetCrossSectionBiasingFactor(globalXsecFactor_);
        theDarkBremProcess->SetMethod(bremMethod_);
        theDarkBremProcess->SetMadGraphDataFile(madGraphFilePath_);

    	G4Electron::ElectronDefinition()->GetProcessManager()->AddProcess(
                theDarkBremProcess /*process to add - G4ProcessManager cleans up processes*/
                , G4ProcessVectorOrdering::ordInActive /*activation when particle at rest*/
                , 1 /*activation along step*/
                , 1 /*activation at end of step*/
                );

    }

}
