/**
 * @file USteppingAction.h
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USTEPPINGACTION_H
#define SIMCORE_USTEPPINGACTION_H

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4UserSteppingAction.hh"

#include "SimPlugins/PluginManagerAccessor.h"

namespace ldmx {

    /**
     * @class USteppingAction
     * @brief Implements the Geant4 user stepping action.
     */
    class USteppingAction : public G4UserSteppingAction, public PluginManagerAccessor {

        public:

            /// Destructor
            ~USteppingAction() final override {;}

            /**
             * Callback used to process a step. 
             * @param step The Geant4 step. 
             */
            void UserSteppingAction(const G4Step* step) final override;
    
    }; // USteppingAction

} // ldmx 

#endif // SIMCORE_USTEPPINGACTION_H
