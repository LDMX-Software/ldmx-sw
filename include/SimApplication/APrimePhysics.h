/**
 * @file APrimePhysics.h
 * @brief Class which defines basic APrime physics
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_APRIMEPHYSICS_H_
#define SIMAPPLICATION_APRIMEPHYSICS_H 1_

// Geant4
#include "G4VPhysicsConstructor.hh"
#include "G4Decay.hh"
#include "G4hMultipleScattering.hh"
#include "G4ProcessManager.hh"
#include "G4Electron.hh"

// Sim Core
#include "SimCore/G4APrime.h"
#include "SimCore/G4eDarkBremsstrahlung.h"

namespace ldmx {

    /**
     * @class APrimePhysics
     * @brief Defines basic APrime physics
     *
     * @note
     * This class basically does not do anything except define
     * a dummy particle so that event generation works properly.
     */
    class APrimePhysics : public G4VPhysicsConstructor {

        public:

            /**
             * Class constructor.
             * @param name The name of the physics.
             */
            APrimePhysics(const G4String& name = "APrime");

            /**
             * Class destructor.
             */
            virtual ~APrimePhysics();

            /**
             * Construct particles.
             */
            void ConstructParticle();

            /**
             * Construct the process.
             */
            void ConstructProcess();

        private:

            /**
             * Definition of the APrime particle.
             */
            G4ParticleDefinition* aprimeDef_;
            //G4Decay decayProcess;
            //G4hMultipleScattering scatterProcess;
    };

}

#endif
