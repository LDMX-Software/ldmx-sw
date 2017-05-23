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
   
    // Forward declare to avoid circular dependency in headers
    class TargetBremFilter;

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
            G4UIcmdWithoutParameter* killRecoilElectronCmd_{nullptr};

    }; // TargetBremFilterMessenger
}

#endif // BIASING_TARGETBREMFILTERMESSENGER_H

