/**
 * @file GammaPhysics.h
 * @brief Class adding extra gamma particle physics for simulation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_GAMMAPHYSICS_H_
#define SIMAPPLICATION_GAMMAPHYSICS_H_

// Geant4
#include "G4VPhysicsConstructor.hh"
#include "G4GammaConversionToMuons.hh"

namespace ldmx {

    /**
     * @class GammaPhysics
     * @brief Adds extra gamma particle physics for simulation
     *
     * @note
     * Currently adds gamma -> mumu reaction using the
     * <i>G4GammaConversionToMuons</i> process.
     */
    class GammaPhysics : public G4VPhysicsConstructor {

        public:

            /**
             * Class constructor.
             * @param name The name of the physics.
             */
            GammaPhysics(const G4String& name = "GammaPhysics");

            /**
             * Class destructor.
             */
            virtual ~GammaPhysics();

            /**
             * Construct particles (no-op).
             */
            void ConstructParticle() {
            }

            /**
             * Construct the process (gamma to muons).
             */
            void ConstructProcess();

        private:

            /**
             * The gamma to muons process.
             */
            G4GammaConversionToMuons gammaConvProcess;
    };

}

#endif
