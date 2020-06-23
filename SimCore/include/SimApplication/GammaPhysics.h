/**
 * @file GammaPhysics.h
 * @brief Class used to enhanced the gamma physics list.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_GAMMAPHYSICS_H_
#define SIMAPPLICATION_GAMMAPHYSICS_H 1_

//------------//
//   Geant4   //
//------------//
#include "G4GammaConversionToMuons.hh"
#include "G4VPhysicsConstructor.hh"
#include "G4VProcess.hh"
#include "G4ProcessManager.hh"

namespace ldmx {

    /**
     * @class GammaPhysics
     * @brief Adds extra gamma particle physics for simulation
     *
     * @note
     * Currently adds gamma -> mumu reaction using the
     * <i>G4GammaConversionToMuons</i> process. Also changes ordering of 
     * gamma processes such that photonNuclear and GammaToMuMu are called first.
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
