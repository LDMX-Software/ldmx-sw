/**
 * @file BiasingMessenger.h
 * @brief Messenger used to pass physics biasing parameters.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_BIASING_MESSENGER_H_
#define SIMAPPLICATION_BIASING_MESSENGER_H_

//------------//
//   Geant4   //
//------------//
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UImessenger.hh"

//----------//
//   LDMX   //
//----------//
#include "SimApplication/RunManager.h"

//namespace ldmx { 

class BiasingMessenger : public G4UImessenger { 

    public: 

        /** Constructor */
        BiasingMessenger();

        /** Destructor */
        ~BiasingMessenger();

        /** */
        void SetNewValue(G4UIcommand* command, G4String newValues);

        /** 
         */
        bool isBiasingEnabled() { return _biasingEnabled; }; 

    private: 

        /** Directory containing all biasing commands. */
        G4UIdirectory* biasingDir_{new G4UIdirectory{"/ldmx/biasing/"}};
        
        /** Command enabling biasing. */
        G4UIcmdWithoutParameter* enableBiasingCmd_{new G4UIcmdWithoutParameter{"/ldmx/biasing/enable", this}};

        /** Command allowing a user to specify what particle type to bias. */
        G4UIcmdWithAString* particleTypeCmd_{new G4UIcmdWithAString{"/ldmx/biasing/particle", this}};
};

        /** Flag indicating if biasing is enabled */
        bool _biasingEnabled{false};

//}

#endif
