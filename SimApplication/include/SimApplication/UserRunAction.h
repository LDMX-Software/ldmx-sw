/**
 * @file UserRunAction.h
 * @brief Class which implements user run action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USERRUNACTION_H_
#define SIMAPPLICATION_USERRUNACTION_H_

// LDMX
#include "SimPlugins/PluginManagerAccessor.h"

// Geant4
#include "G4UserRunAction.hh"
#include "G4Run.hh"

namespace ldmx {

    /**
     * @class UserRunAction
     * @brief Implementation of user run action hook
     */
    class UserRunAction : public G4UserRunAction, public PluginManagerAccessor {

        public:

            /**
             * Class constructor.
             */
            UserRunAction();

            /**
             * Class destructor.
             */
            virtual ~UserRunAction();

            /**
             * Implementation of begin run hook.
             * @param aRun The current Geant4 run info.
             */
            void BeginOfRunAction(const G4Run* aRun);

            /**
             * Implementation of end run hook.
             * @param aRun The current Geant4 run info.
             */
            void EndOfRunAction(const G4Run* aRun);
    };

}

#endif
