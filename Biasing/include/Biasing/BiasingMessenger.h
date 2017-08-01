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

namespace ldmx { 

    class BiasingMessenger : public G4UImessenger { 

        public: 

            /** Constructor */
            BiasingMessenger();

            /** Destructor */
            ~BiasingMessenger();

            /** Set the event weight. */
            static void setEventWeight(double eventWeight) { eventWeight_ = eventWeight; };

            /** */
            void SetNewValue(G4UIcommand* command, G4String newValues);

            /** Method to indicate whether biasing is enabled. */
            static bool isBiasingEnabled() { return biasingEnabled_; };

            /** 
             * Get the event weight.  The weight is corrected to take into 
             * account the biased cross-section. 
             */
            static double getEventWeight() { return eventWeight_; }; 

            /** 
             * Get the particle type (e.g. gamma, e-) to which the biasing is
             * being applied. 
             */
            static std::string getParticleType() { return particleType_; };

            /** Get the process (e.g. photonNuclear) that is being processed. */
            static std::string getProcess() { return process_; }; 

            /**
             * Get the energy threshold required for the biasing operation to 
             * be applied.
             */
            static double getThreshold() { return threshold_; }; 

            /** Get the volume to which the biasing operation is attached to. */
            static std::string getVolume() { return volume_; };

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

            /** Flag indicating if biasing is enabled */
            static bool biasingEnabled_;

            /** Event weight corrected to account for biased cross-section. */
            static double eventWeight_;

            /** Particle specifies to bias. */
            static std::string particleType_;

            /** Process to bias. */
            static std::string process_;

            /** Particle energy threshold. */
            static double threshold_;

            /** Volume to attach biasing to. */
            static std::string volume_;

    }; // BiasingMessenger
}

#endif // SIMAPPLICATION_BIASING_MESSENGER_H
