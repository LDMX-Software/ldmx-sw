/**
 * @file EcalProcessFilterMessenger.h
 * @brief Messenger for setting parameters on EcalProcessFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_ECALPROCESSFILTERMESSENGER_H
#define BIASING_ECALPROCESSFILTERMESSENGER_H

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithAString.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Biasing/EcalProcessFilter.h"
#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx { 
   
    // Forward declare to avoid circular dependency in headers
    class EcalProcessFilter;

    class EcalProcessFilterMessenger : UserActionPluginMessenger {
        
        public: 

            /** 
             * Constructor
             *
             * @param Filter associated with this messenger.
             */
            EcalProcessFilterMessenger(EcalProcessFilter* filter); 

            /** Destructor */
            ~EcalProcessFilterMessenger();

            /**
             */
            void SetNewValue(G4UIcommand * command, G4String newValue);

        private:

            /** The filter associated with this messenger. */
            EcalProcessFilter* filter_{nullptr};
            
            /** 
             * Command allowing a user to specify what volume the filter 
             * should be applied to.
             */
            G4UIcmdWithAString* volumeCmd_{nullptr};

            /** 
             * Command allowing a user to specify whether a particle should 
             * be bound to the specified volume.  If so, once the particle
             * exits the volume it will be killed. 
             */
            G4UIcmdWithAString* boundCmd_{nullptr};


    }; // EcalProcessFilterMessenger
}

#endif // BIASING_ECALPROCESSFILTERMESSENGER_H

