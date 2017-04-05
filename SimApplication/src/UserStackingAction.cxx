#include "SimApplication/UserStackingAction.h"

#include "SimPlugins/PluginManager.h"

namespace ldmx {

    G4ClassificationOfNewTrack UserStackingAction::ClassifyNewTrack(const G4Track *aTrack) {
        return pluginManager_->stackingClassifyNewTrack(aTrack);
    }

    void UserStackingAction::NewStage() {
        pluginManager_->stackingNewStage();
    }

    void UserStackingAction::PrepareNewEvent() {
        pluginManager_->stackingPrepareNewEvent();
    }

}
