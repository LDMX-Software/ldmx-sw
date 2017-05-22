/**
 * @file RunManager.h
 * @brief Class providing a Geant4 run manager implementation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_RUNMANAGER_H_
#define SIMAPPLICATION_RUNMANAGER_H_

// Geant4
#include "G4Decay.hh"
#include "G4hMultipleScattering.hh"
#include "G4GDMLParser.hh"
#include "G4RunManager.hh"

// LDMX
#include "Biasing/BiasingMessenger.h"
#include "SimPlugins/PluginMessenger.h"
#include "SimApplication/ParallelWorldMessenger.h"
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/ParallelWorld.h"

namespace ldmx {

    // Forward declaration
    //class DetectorConstruction;

    /**
     * @class RunManager
     * @brief Extension of Geant4 run manager
     */
    class RunManager : public G4RunManager {

        public:

            /**
             * Class constructor.
             */
            RunManager();

            /**
             * Class destructor.
             */
            virtual ~RunManager();

            /**
             * Initialize physics.
             */
            void InitializePhysics();

            /**
             * Perform application initialization.
             */
            void Initialize();

            /**
             * Get the user detector construction cast to a specific type.
             * @return The user detector construction.
             */
            DetectorConstruction* getDetectorConstruction() {
                return (DetectorConstruction*) this->userDetector;
            }

        private:

            /** Plugin messenger. */
            PluginMessenger* pluginMessenger_;

            /** Biasing messenger. */
            BiasingMessenger* biasingMessenger_ {new BiasingMessenger()};

            /** Parallel world messenger. */
            ParallelWorldMessenger* pwMessenger_{new ParallelWorldMessenger()};

            /**
             * Manager of sim plugins.
             */
            PluginManager* pluginManager_;

    };
// RunManager

}

#endif // SIMAPPLICATION_RUNMANAGER_H_
