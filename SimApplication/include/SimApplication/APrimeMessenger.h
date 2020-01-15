/**
 * @file APrimeMessnger.h
 * @brief Messenger for setting parallel world parameters.
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMAPPLICATION_APRIMEMESSENGER_H_
#define SIMAPPLICATION_APRIMEMESSENGER_H_

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UImessenger.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/RunManager.h"

namespace ldmx { 

    // Forward declare to avoid circular depedency in headers
    class RunManager;

    class APrimeMessenger : public G4UImessenger { 
        
        public: 

            /** Constructor */
            APrimeMessenger(RunManager* runManager);

            /** Destructor */
            ~APrimeMessenger(); 
            
            /** */
            void SetNewValue(G4UIcommand* command, G4String newValues);

        private: 

            /** Run manager */
            RunManager* runManager_{nullptr};

            /** Directory containing all of the aprime commands. */
            G4UIdirectory* aprimeDir_{nullptr};

            /** Mass of the A' for this run */
            G4UIcmdWithADoubleAndUnit* massCmd_{nullptr};

    }; // APrimeMessenger
}

#endif // SIMAPPLICATION_APRIMEMESSENGER_H_
