/**
 * @file TargetDarkBremFilterMessenger.h
 * @brief Messenger for setting parameters on TargetDarkBremFilter.
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
#include "Biasing/TargetDarkBremFilter.h"
#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx { 
   
    // Forward declare to avoid circular dependency in headers
    class TargetDarkBremFilter;

    class TargetDarkBremFilterMessenger : UserActionPluginMessenger {
        
        public: 

            /** 
             * Constructor
             *
             * @param Filter associated with this messenger.
             */
            TargetDarkBremFilterMessenger(TargetDarkBremFilter* filter); 

            /** Destructor */
            ~TargetDarkBremFilterMessenger();

            /**
             */
            void SetNewValue(G4UIcommand * command, G4String newValue);

        private:

            /** The filter associated with this messenger. */
            TargetDarkBremFilter* filter_{nullptr};
            
            /** Command dictating whether the electron track gets killed. */
            G4UIcmdWithoutParameter* killRecoilCmd_{nullptr};

            /** 
             * Command allowing a user to specify what volume the filter 
             * should be applied to.
             */
            G4UIcmdWithAString* volumeCmd_{nullptr};

    }; // TargetDarkBremFilterMessenger
}

#endif // BIASING_TARGETBREMFILTERMESSENGER_H

