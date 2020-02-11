/**
 * @file USteppingAction.cxx
 * @author Omar Moreno, SLAC National Accelerator Laboraty
 */

#include "SimApplication/USteppingAction.h" 

#include "SimPlugins/PluginManager.h"

namespace ldmx {

    void USteppingAction::UserSteppingAction(const G4Step* aStep) {
        pluginManager_->stepping(aStep);
    }

} // ldmx 
