/**
 * @file ParallelWorldMessnger.h
 * @brief Messenger for setting parallel world parameters.
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

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/RunManager.h"

namespace ldmx { 

    // Forward declare to avoid circular depedency in headers
    class RunManager;

    class ParallelWorldMessenger : public G4UImessenger { 
        
        public: 

            /** Constructor */
            ParallelWorldMessenger(RunManager* runManager);

            /** Destructor */
            ~ParallelWorldMessenger(); 
            
            /** */
            void SetNewValue(G4UIcommand* command, G4String newValues);

        private: 

            /** Run manager */
            RunManager* runManager_{nullptr};

            /** Directory containing all of the parallel world commands. */
            G4UIdirectory* pwDir_{new G4UIdirectory{"/ldmx/pw/"}};

            /** Command enabling the use of parallel worlds. */
            G4UIcmdWithoutParameter* enablePWCmd_{new G4UIcmdWithoutParameter{"/ldmx/pw/enable", this}};

            /** Path to GDML file containing the detector description. */
            G4UIcmdWithAString* readCmd_{new G4UIcmdWithAString{"/ldmx/pw/read", this}};

    }; // ParallelWorldMessenger
}

#endif // SIMAPPLICATION_PARALLELWORLDMESSENGER_H_
