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
            }
            ;

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

            /**
             * FIXME: This should not be static.
             */
            static bool useRootSeed_;

    };

}

#endif
