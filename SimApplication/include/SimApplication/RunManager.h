/**
 * @file RunManager.h
 * @brief Class providing a Geant4 run manager implementation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_RUNMANAGER_H_
#define SIMAPPLICATION_RUNMANAGER_H_

// Geant4
#include "G4RunManager.hh"
#include "G4hMultipleScattering.hh"
#include "G4Decay.hh"

// LDMX
#include "Biasing/BiasingMessenger.h"
#include "SimPlugins/PluginMessenger.h"

namespace ldmx {

    // Forward declaration
    class DetectorConstruction;

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

            /**
             * The plugin messenger.
             */
            PluginMessenger* pluginMessenger_;

            /**
             * Biasing messenger.
             */
            BiasingMessenger* biasingMessenger_ {new BiasingMessenger()};

            /**
             * Manager of sim plugins.
             */
            PluginManager* pluginManager_;

    };
// RunManager

}

#endif // SIMAPPLICATION_RUNMANAGER_H_
