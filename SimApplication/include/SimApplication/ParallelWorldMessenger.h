/**
 * @file ParallelWorldMessnger.h
 * @brief Messenger used to pass parameters used to setup a parallel world.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_PARALLELWORLDMESSENGER_H_
#define SIMAPPLICATION_PARALLELWORLDMESSENGER_H_

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UImessenger.hh"

namespace ldmx { 

    class ParallelWorldMessenger : public G4UImessenger { 
        
        public: 

            /** Constructor */
            ParallelWorldMessenger();

            /** Destructor */
            ~ParallelWorldMessenger(); 
            
            /** */
            void SetNewValue(G4UIcommand* command, G4String newValues);

            /**
             * Method used to check if the use of a parallel world has been 
             * enabled.
             */
            static bool isParallelWorldEnabled() { return enableParallelWorld_; }; 
        
            /**
             * @return The path to the GDML file containing the detector
             *         description. 
             */
            static std::string getDetectorPath() { return gdmlPath_; };

        private: 

            /** Directory containing all of the parallel world commands. */
            G4UIdirectory* pwDir_{new G4UIdirectory{"/ldmx/pw/"}};

            /** Command enabling the use of parallel worlds. */
            G4UIcmdWithoutParameter* enablePWCmd_{new G4UIcmdWithoutParameter{"/ldmx/pw/enable", this}};

            /** Path to GDML file containing the detector description. */
            G4UIcmdWithAString* readCmd_{new G4UIcmdWithAString{"/ldmx/pw/read", this}};

            /** Flag indicating if a parallel world should be loaded. */
            static bool enableParallelWorld_;

            /** Path to GDML file containing the detector description. */
            static std::string gdmlPath_; 

    }; // ParallelWorldMessenger
}

#endif // SIMAPPLICATION_PARALLELWORLDMESSENGER_H_
