/**
 * @file PrimaryGeneratorMessenger.h
 * @brief Class providing a macro messenger for event generation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_
#define SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"

// LDMX
#include "SimApplication/PrimaryGeneratorAction.h"

namespace ldmx {

    /**
     * @class PrimaryGeneratorMessenger
     * @brief Macro messenger for event generation
     */
    class PrimaryGeneratorMessenger : public G4UImessenger {

        public:

            /**
             * Class constructor.
             * @param pga The primary generator action.
             */
            PrimaryGeneratorMessenger(PrimaryGeneratorAction* pga);

            /**
             * Class destructor.
             */
            virtual ~PrimaryGeneratorMessenger();

            /**
             * Process macro command.
             * @param command The applicable UI command.
             * @param newValues The argument values.
             */
            void SetNewValue(G4UIcommand* command, G4String newValues);

            /** 
             */
            static bool useRootSeed() {
                return useRootSeed_;
            };

            /** Get the particle type (e.g. gamma, e-) for mpg */
            static std::string getMPGParticleType() { return particleType_; };
        
            /** Get the particle energy for mpg */
            static double getMPGParticleEnergy() { return particleEnergy_; };

            /** Get the particle energy for mpg */
            static int getMPGnInteractions() { return nInteractions_; };

            static bool useBeamspot() { return useBeamspot_; };
            /** Get the particle energy for mpg */
            static double getBeamspotSize() { return beamspotSize_; };

        private:

            /**
             * The primary generator action.
             */
            PrimaryGeneratorAction* primaryGeneratorAction_;

            /**
             * The LHE generator macro directory.
             */
            G4UIdirectory* lheDir_;

            /**
             * The command for opening LHE files.
             */
            G4UIcommand* lheOpenCmd_;

            /**
             * The Root generator macro directory.
             */
            G4UIdirectory* rootDir_;

            /**
             * The command for opening Root files.
             */
            G4UIcommand* rootOpenCmd_;

            /** 
             * The command for opening Root files.
             */
            G4UIcmdWithoutParameter* rootUseSeedCmd_ {new G4UIcmdWithoutParameter {"/ldmx/generators/root/useSeed", this}};

            /** The command for using the multiparticle gun. */
            G4UIcmdWithoutParameter* enableMPGunCmd_ {new G4UIcmdWithoutParameter {"/ldmx/generators/mpgun/enable", this}};
            /** Command allowing a user to specify what particle type to generate. */
            G4UIcmdWithAString* mpgunParticleTypeCmd_{new G4UIcmdWithAString{"/ldmx/generators/mpgun/particle", this}};
            /** Command allowing a user to specify what particle type to generate. */
            G4UIcmdWithAString* mpgunEnergyCmd_{new G4UIcmdWithAString{"/ldmx/generators/mpgun/energy", this}};
            /** Command allowing a user to specify what particle type to generate. */
            G4UIcmdWithAString* mpgunNIntCmd_{new G4UIcmdWithAString{"/ldmx/generators/mpgun/nInteractions", this}};

            /** Command allowing a user to specify what particle type to generate. */
            G4UIcmdWithoutParameter* enableBeamspotCmd_{new G4UIcmdWithoutParameter{"/ldmx/generators/beamspot/enable", this}};
            /** Command allowing a user to specify what particle type to generate. */
            G4UIcmdWithAString* beamspotSizeCmd_{new G4UIcmdWithAString{"/ldmx/generators/beamspot/size", this}};

            /**
             * FIXME: This should not be static.
             */
            static bool useRootSeed_;
            // /** particle type */
            static std::string particleType_;
            // * Particle energy threshold. 
            static double particleEnergy_;
            // * Particle energy threshold. 
            static int nInteractions_;

            // * Particle energy threshold. 
            static bool useBeamspot_;
            // * Particle energy threshold. 
            static double beamspotSize_;

    };

}

#endif
