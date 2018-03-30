/**
 * @file TargetENProcessFilterMessenger.h
 * @brief Messenger for setting parameters on TargetENProcessFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_TARGETENPROCESSFILTERMESSENGER_H
#define BIASING_TARGETENPROCESSFILTERMESSENGER_H

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithoutParameter.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Biasing/TargetENProcessFilter.h"
#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx { 
   
    // Forward declare to avoid circular dependency in headers
    class TargetENProcessFilter;

    class TargetENProcessFilterMessenger : UserActionPluginMessenger {
        
        public: 

            /** 
             * Constructor
             *
             * @param Filter associated with this messenger.
             */
            TargetENProcessFilterMessenger(TargetENProcessFilter* filter); 

            /** Destructor */
            ~TargetENProcessFilterMessenger();

            /**
             */
            void SetNewValue(G4UIcommand * command, G4String newValue);

        private:

            /** The filter associated with this messenger. */
            TargetENProcessFilter* filter_{nullptr};
            
            /**
             * Command allowing a user to specify the energy threshold that the
             * recoil electron must not exceed. 
             */
            G4UIcmdWithAString* recoilEnergyThresholdCmd_{nullptr};

            /** 
             * Command allowing a user to specify what volume the filter 
             * should be applied to.
             */
            G4UIcmdWithAString* volumeCmd_{nullptr};

    }; // TargetENProcessFilterMessenger
}

#endif // BIASING_TARGETENPROCESSFILTERMESSENGER_H

