#include "SimPlugins/PluginManager.h"
#include "../include/SimApplication/SteppingAction.h"

namespace ldmx {

void SteppingAction::UserSteppingAction(const G4Step* aStep) {
    pluginManager_->stepping(aStep);
}

}
