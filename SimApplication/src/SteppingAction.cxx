#include "SimPlugins/PluginManager.h"
#include "../include/SimApplication/SteppingAction.h"

namespace sim {

void SteppingAction::UserSteppingAction(const G4Step* aStep) {
    pluginManager_->stepping(aStep);
}

}
