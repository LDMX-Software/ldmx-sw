/**
 * @file XsecBiasingOperatorMessenger.h
 * @brief Messenger for setting parameters on XsecBiasingOperator.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_XSECBIASINGOPERATORMESSENGER_H
#define BIASING_XSECBIASINGOPERATORMESSENGER_H

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UImessenger.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/XsecBiasingOperator.h"

namespace ldmx { 
   
    // Forward declare to avoid circular dependency in headers
    class XsecBiasingOperator;

    class XsecBiasingOperatorMessenger : G4UImessenger {
        
        public: 

            /** 
             * Constructor
             *
             * @param Operator associated with this messenger.
             */
            XsecBiasingOperatorMessenger(XsecBiasingOperator* oprt); 

            /** Destructor */
            ~XsecBiasingOperatorMessenger();

            /**
             */
            void SetNewValue(G4UIcommand * command, G4String newValue);

        private:

            /** The operator associated with this messenger. */
            XsecBiasingOperator* operator_{nullptr};

            /** Directory containing all biasing commands. */
            G4UIdirectory* biasingDir_{nullptr}; 
           
            /** 
             * Command used to disable the biasing down of the EM cross-section
             * when either the photonuclear or gamma->mu+mu- is biased up.
             */
            G4UIcmdWithoutParameter* biasDownEMCmd_{nullptr};

            /** 
             * Command dictating whether all particles of the given type
             * should be biased. 
             */
            G4UIcmdWithoutParameter* biasAllCmd_{nullptr}; 

            /** 
             * Command dictating whether only the incident particle of 
             * the given type should be biased. 
             */
            G4UIcmdWithoutParameter* biasIncidentCmd_{nullptr}; 

            /** 
             * Command allowing a user to specify what particle type to 
             * bias.
             */
            G4UIcmdWithAString* particleTypeCmd_{nullptr};

            /** Command allowing a user to specify an energy threshold. */
            G4UIcmdWithAString* thresholdCmd_{nullptr};

            /** 
             * Command allowing a user to specify by what factor the 
             * xsec of the process will be increased. 
             */
            G4UIcmdWithAString* xsecTransCmd_{nullptr};
    
    }; // XsecBiasingOperatorMessenger
}

#endif // BIASING_XSECBIASINGOPERATORMESSENGER_H

