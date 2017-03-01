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
            static bool isBiasingEnabled() { return biasingEnabled_; }; 

            /**
            */
            static std::string getParticleType() { return particleType_; };

            /**
            */
            static std::string getProcess() { return process_; }; 

            /**
             */
            static double getThreshold() { return threshold_; }; 

            /**
            */
            static std::string getVolume() { return volume_; };

            /**
            */
            static double getXsecTrans() { return xsecTrans_; }; 

        private: 

            /** Directory containing all biasing commands. */
            G4UIdirectory* biasingDir_{new G4UIdirectory{"/ldmx/biasing/"}};

            /** Command enabling biasing. */
            G4UIcmdWithoutParameter* enableBiasingCmd_{new G4UIcmdWithoutParameter{"/ldmx/biasing/enable", this}};

            /** Command allowing a user to specify what particle type to bias. */
            G4UIcmdWithAString* particleTypeCmd_{new G4UIcmdWithAString{"/ldmx/biasing/particle", this}};

            /** Command allowing a user to specify what process to bias. */
            G4UIcmdWithAString* processCmd_{new G4UIcmdWithAString{"/ldmx/biasing/process", this}};

            /** Command allowing a user to specify an energy threshold. */
            G4UIcmdWithAString* thresholdCmd_{new G4UIcmdWithAString{"/ldmx/biasing/threshold", this}};

            /** Command allowing a user to specify what volume the biasing should be attached to. */
            G4UIcmdWithAString* volumeCmd_{new G4UIcmdWithAString{"/ldmx/biasing/volume", this}};

            /** Command allowing a user to specify by what factor the xsec of the process will be increased. */
            G4UIcmdWithAString* xsecTransCmd_{new G4UIcmdWithAString{"/ldmx/biasing/xsec", this}};

            /** Flag indicating if biasing is enabled */
            static bool biasingEnabled_;

            /** Particle specifies to bias. */
            static std::string particleType_;

            /** Process to bias. */
            static std::string process_;

            /** Particle energy threshold. */
            static double threshold_;

            /** Volume to attach biasing to. */
            static std::string volume_;

            /** Factor to multiple the xsec by. */
            static double xsecTrans_;

    }; // BiasingMessenger

}

#endif // SIMAPPLICATION_BIASING_MESSENGER_H
