/**
 * @file TargetBremFilterMessenger.h
 * @brief Messenger for setting parameters on TargetBremFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_TARGETBREMFILTERMESSENGER_H
#define BIASING_TARGETBREMFILTERMESSENGER_H

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithoutParameter.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Biasing/TargetBremFilter.h"
#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx { 
    
    class TargetBremFilterMessenger : UserActionPluginMessenger {
        
        public: 

            /** 
             * Constructor
             *
             * @param Filter associated with this messenger.
             */
            TargetBremFilterMessenger(TargetBremFilter* filter); 

            /** Destructor */
            ~TargetBremFilterMessenger();

            /**
             */
            void SetNewValue(G4UIcommand * command, G4String newValue);

        private:

            /** The filter associated with this messenger. */
            TargetBremFilter* filter_{nullptr};

            /** Command dictating whether the electron track gets killed. */
            G4UIcmdWithoutParameter* killElectronCmd_{nullptr};

            /** Flag indicating whether the electron track gets killed. */
            bool killElectron_{false};

    }; // TargetBremFilterMessenger
}

#endif // BIASING_TARGETBREMFILTERMESSENGER_H

