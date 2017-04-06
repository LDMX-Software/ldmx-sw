/**
 * @file SteppingAction.h
 * @brief Class implementing the Geant4 user stepping action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USERSTEPPINGACTION_H_
#define SIMAPPLICATION_USERSTEPPINGACTION_H_

// LDMX
#include "SimPlugins/PluginManagerAccessor.h"

// Geant4
#include "G4UserSteppingAction.hh"

namespace ldmx {

    /**
     * @class SteppingAction
     * @brief Implements the Geant4 user stepping action
     */
    class SteppingAction : public G4UserSteppingAction, public PluginManagerAccessor {

        public:

            /**
             * Class destructor.
             */
            virtual ~SteppingAction() {;}

            /**
             * Process a step.
             * @param aStep The step information.
             */
            void UserSteppingAction(const G4Step* aStep);
    };

}

#endif
