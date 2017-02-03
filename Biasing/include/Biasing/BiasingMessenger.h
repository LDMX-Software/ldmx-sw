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

namespace ldmx { 

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
            bool isBiasingEnabled() { return biasingEnabled_; }; 

            /**
            */
            std::string getParticleType() { return particleType_; };

            /**
            */
            std::string getProcess() { return process_; }; 

            /**
            */
            std::string getVolume() { return volume_; };

            /**
            */
            double getXsecTrans() { return xsecTrans_; }; 

        private: 

            /** Directory containing all biasing commands. */
            G4UIdirectory* biasingDir_{new G4UIdirectory{"/ldmx/biasing/"}};

            /** Command enabling biasing. */
            G4UIcmdWithoutParameter* enableBiasingCmd_{new G4UIcmdWithoutParameter{"/ldmx/biasing/enable", this}};

            /** Command allowing a user to specify what particle type to bias. */
            G4UIcmdWithAString* particleTypeCmd_{new G4UIcmdWithAString{"/ldmx/biasing/particle", this}};

            /** Command allowing a user to specify what process to bias. */
            G4UIcmdWithAString* processCmd_{new G4UIcmdWithAString{"/ldmx/biasing/process", this}};

            /** Command allowing a user to specify what volume the biasing should be attached to. */
            G4UIcmdWithAString* volumeCmd_{new G4UIcmdWithAString{"/ldmx/biasing/volume", this}};

            /** Command allowing a user to specify by what factor the xsec of the process will be increased. */
            G4UIcmdWithAString* xsecTransCmd_{new G4UIcmdWithAString{"/ldmx/biasing/xsec", this}};

            /** Flag indicating if biasing is enabled */
            bool biasingEnabled_{false};

            /** Particle specifies to bias. */
            std::string particleType_{"gamma"};

            /** Process to bias. */
            std::string process_{"photonNuclear"};

            /** Volume to attach biasing to. */
            std::string volume_{"target"};

            /** Factor to multiple the xsec by. */
            double xsecTrans_{1.0};

    }; // BiasingMessenger

}

#endif // SIMAPPLICATION_BIASING_MESSENGER_H
