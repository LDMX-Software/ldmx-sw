/**
 * @file RunManager.h
<<<<<<< HEAD
 * @brief Class providing a Geant4 run manager implementation.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _SIMAPPLICATION_RUNMANAGER_H_
#define _SIMAPPLICATION_RUNMANAGER_H_

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Biasing/BiasingMessenger.h"

namespace ldmx {

    // Forward declare to avoid circular dependency in headers
    class DetectorConstruction; 
    class ParallelWorldMessenger; 
    class PluginManager; 
    class PluginMessenger; 

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
            DetectorConstruction* getDetectorConstruction(); 

            /** Enable a parallel world. */
            void enableParallelWorld(bool isPWEnabled) { isPWEnabled_ = isPWEnabled; }

            /** Set the path to the GDML description of the parallel world. */
            void setParallelWorldPath(std::string parallelWorldPath) { 
                parallelWorldPath_ = parallelWorldPath; 
            }

        private:

            /** Plugin messenger. */
            PluginMessenger* pluginMessenger_;

=======
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

>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
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
<<<<<<< HEAD

    }; // RunManager
} // ldmx

#endif // _SIMAPPLICATION_RUNMANAGER_H_
=======
    };
// RunManager

}

#endif // SIMAPPLICATION_RUNMANAGER_H_
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
