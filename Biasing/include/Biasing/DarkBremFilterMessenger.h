/**
 * @file DarkBremFilterMessenger.h
 * @brief Messenger for setting parameters on DarkBremFilter.
 * @author Michael Revering, University of Minnesota
 */

#ifndef BIASING_DARKBREMFILTERMESSENGER_H
#define BIASING_DARKBREMFILTERMESSENGER_H

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithoutParameter.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Biasing/DarkBremFilter.h"
#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx { 
   
    // Forward declare to avoid circular dependency in headers
    class DarkBremFilter;

    /**
     * @class DarkBremFilterMessenger
     *
     * Interface between geant4 macro commands and the DarkBremFilter.
     */
    class DarkBremFilterMessenger : UserActionPluginMessenger {
        
        public: 

            /** 
             * Constructor
             *
             * @param Filter associated with this messenger.
             */
            DarkBremFilterMessenger(DarkBremFilter* filter); 

            /** Destructor */
            ~DarkBremFilterMessenger();

            /**
             * Copies input value to DarkBremFilter.
             *
             * Right now only one command available: setting the physical volume for the filter.
             */
            void SetNewValue(G4UIcommand * command, G4String newValue);

        private:

            /** The filter associated with this messenger. */
            DarkBremFilter* filter_{nullptr};
            
            /** 
             * Command allowing a user to specify what volume the filter 
             * should be applied to.
             */
            G4UIcmdWithAString* volumeCmd_{nullptr};

    }; // DarkBremFilterMessenger
}

#endif // BIASING_DARKBREMFILTERMESSENGER_H

