/**
 * @file RunManager.h
 * @brief Class providing a Geant4 run manager implementation.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_RUNMANAGER_H
#define SIMAPPLICATION_RUNMANAGER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <map>
#include <string>

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

class G4PhysListFactory; 

namespace ldmx {

    // Forward declare to avoid circular dependency in headers
    class DetectorConstruction;
    class UserActionManager; 
    class APrimeMessenger;

    /**
     * @class RunManager
     * @brief Extension of Geant4 run manager
     */
    class RunManager : public G4RunManager {

        public:

            /**
             * Class constructor.
             */
            RunManager(Parameters& parameters); 

            /**
             * Class destructor.
             */
            virtual ~RunManager();

            /**
             * Initialize physics.
             */
            void setupPhysics();

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

            /** Define A Prime mass for this simulation run */
            void setAPrimeMass(double theMass) { aPrimeMass_ = theMass; }

            /** Give path to mad graph events file */
            void setMadGraphFilePath(const std::string& filepath) { madGraphFilePath_ = filepath; }

            /** Give the method of interpreting mad graph events */
            void setDarkBremMethod(int method) { bremMethod_ = method; }

            /** Set global xsec biasing factor for DarkBrem process */
            void setGlobalXsecFactor(double factor) { globalXsecFactor_ = factor; }

            /** Set the path to the GDML description of the parallel world. */
            void setParallelWorldPath(std::string parallelWorldPath) { 
                parallelWorldPath_ = parallelWorldPath; 
            }

            /**
             * Tell RunManager to use the seed from the root file.
             */
            void setUseRootSeed(bool useIt = true) { useRootSeed_ = useIt; }

            /**
             * Should we use the seed from the root file?
             */
            bool useRootSeed() { return useRootSeed_; }
 
        private:

            /// The set of parameters used to configure the RunManager
            Parameters parameters_; 

            /**
             * Factory class for instantiating the physics list.
             */
            G4PhysListFactory* physicsListFactory_{nullptr};

            /** 
             * Flag indicating whether a parallel world should be 
             * registered 
             */
            bool isPWEnabled_{false};

            /**
             * Mass of A Prime this simulation should use.
             *
             * Negative value means no dark brems.
             */
            double aPrimeMass_{-1};

            /**
             * File path to mad graph events simulation dark brems
             */
            std::string madGraphFilePath_;

            /**
             * Method to interpret the mad graph data
             */
            int bremMethod_;

            /**
             * Global xsec biasing factor to apply to the dark brem
             */
            double globalXsecFactor_{1.};

            /** Path to GDML description of parallel world. */
            std::string parallelWorldPath_{""};

            /**
             * Should we use random seed from root file?
             */
            bool useRootSeed_{false};

    }; // RunManager
} // ldmx

#endif // _SIMAPPLICATION_RUNMANAGER_H_
