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

    // Forward declare to avoid circular dependency in headers
    class ParallelWorldMessenger; 

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

            /** Enable a parallel world. */
            void enableParallelWorld(bool isPWEnabled) { isPWEnabled_ = isPWEnabled; }

            /** Set the path to the GDML description of the parallel world. */
            void setParallelWorldPath(std::string parallelWorldPath) {
                parallelWorldPath_ = parallelWorldPath; 
            }

        private:

            /** Plugin messenger. */
            PluginMessenger* pluginMessenger_;

            /** Biasing messenger. */
            BiasingMessenger* biasingMessenger_ {new BiasingMessenger()};

            /** Parallel world messenger. */
            ParallelWorldMessenger* pwMessenger_{nullptr};

            /**
             * Manager of sim plugins.
             */
            PluginManager* pluginManager_{nullptr};

            /** 
             * Flag indicating whether a parallel world should be 
             * registered 
             */
            bool isPWEnabled_{false};

            /** Path to GDML description of parallel world. */
            std::string parallelWorldPath_{""};
    };
// RunManager

}

#endif // SIMAPPLICATION_RUNMANAGER_H_
