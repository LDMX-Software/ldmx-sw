/**
 * @file PrimaryGeneratorMessenger.h
 * @brief Class providing a macro messenger for event generation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_
#define SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_

//------------//
//   Geant4   //
//------------//
#include "G4UImessenger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"

//-------------//
//   LDMX-SW   //
//-------------//
#include "SimApplication/LHEPrimaryGenerator.h"
#include "SimApplication/MultiParticleGunPrimaryGenerator.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/RootPrimaryGenerator.h"

namespace ldmx {

    // Forward declare to avoid circular dependency in headers
    class PrimaryGeneratorAction; 

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
            
            /** The command for using the multiparticle gun. */
            G4UIcmdWithoutParameter* enableMPGunPoissonCmd_ {new G4UIcmdWithoutParameter {"/ldmx/generators/mpgun/enablePoisson", this}};
            G4UIcmdWithAString* mpgunNParCmd_ {new G4UIcmdWithAString{"/ldmx/generators/mpgun/nInteractions", this}};
            G4UIcmdWithAString* mpgunVtxCmd_ {new G4UIcmdWithAString{"/ldmx/generators/mpgun/vertex", this}};
            G4UIcmdWithAString* mpgunPIDCmd_ {new G4UIcmdWithAString{"/ldmx/generators/mpgun/pdgID", this}};
            G4UIcmdWithAString* mpgunMomCmd_ {new G4UIcmdWithAString{"/ldmx/generators/mpgun/momentum", this}};

            /** Command allowing a user to specify what particle type to generate. */
            G4UIcmdWithoutParameter* enableBeamspotCmd_ {new G4UIcmdWithoutParameter{"/ldmx/generators/beamspot/enable", this}};

            /** Command allowing a user to specify what particle type to generate. */
            G4UIcmdWithAString* beamspotXSizeCmd_ {new G4UIcmdWithAString{"/ldmx/generators/beamspot/sizeX", this}};
            G4UIcmdWithAString* beamspotYSizeCmd_ {new G4UIcmdWithAString{"/ldmx/generators/beamspot/sizeY", this}};
            G4UIcmdWithAString* beamspotZSizeCmd_ {new G4UIcmdWithAString{"/ldmx/generators/beamspot/sizeZ", this}};

            /**
             * FIXME: This should not be static.
             */
            static bool useRootSeed_;
    
    }; // PrimaryGeneratorMessenger
}  // ldmx
#endif // SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_
