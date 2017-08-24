/**
 * @file SimpleProcessFilterMessenger.h
 * @brief Messenger for setting parameters on SimpleProcessFilter.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_SIMPLEPROCESSFILTERMESSENGER_H
#define BIASING_SIMPLEPROCESSFILTERMESSENGER_H

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithoutParameter.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Biasing/SimpleProcessFilter.h"
#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx { 
   
    // Forward declare to avoid circular dependency in headers
    class SimpleProcessFilter;

    class SimpleProcessFilterMessenger : UserActionPluginMessenger {
        
        public: 

            /** 
             * Constructor
             *
             * @param Filter associated with this messenger.
             */
            SimpleProcessFilterMessenger(SimpleProcessFilter* filter); 

            /** Destructor */
            ~SimpleProcessFilterMessenger();

            /**
             */
            void SetNewValue(G4UIcommand * command, G4String newValue);

        private:

            /** The filter associated with this messenger. */
            SimpleProcessFilter* filter_{nullptr};
            
            /** 
             * Command allowing a user to specify the parent ID of the
             * particle to which the filter will be applied to.
             */
            G4UIcmdWithAString* parentIDCmd_{nullptr};

            /** 
             * Command allowing a user to specify the PDG ID of the
             * particle to which the filter will be applied to.
             */
            G4UIcmdWithAString* trackIDCmd_{nullptr};

            /** 
             * Command allowing a user to specify the PDG ID of the
             * particle to which the filter will be applied to.
             */
            G4UIcmdWithAString* pdgIDCmd_{nullptr};

            /** 
             * Command allowing a user to specify the process to filter on. 
             */
            G4UIcmdWithAString* processCmd_{nullptr};

            /** 
             * Command allowing a user to specify what volume the filter 
             * should be applied to.
             */
            G4UIcmdWithAString* volumeCmd_{nullptr};

    }; // SimpleProcessFilterMessenger
}

#endif // BIASING_SIMPLEPROCESSFILTERMESSENGER_H

