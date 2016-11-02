#include "SimApplication/UserStackingAction.h"

#include "SimPlugins/PluginManager.h"

namespace sim {

G4ClassificationOfNewTrack UserStackingAction::ClassifyNewTrack (const G4Track *aTrack) {
    // The last plugin activated will override all the others!
    return PluginManager::getInstance().stackingClassifyNewTrack(aTrack);
}

void UserStackingAction::NewStage() {
    PluginManager::getInstance().stackingNewStage();
}

void UserStackingAction::PrepareNewEvent() {
    PluginManager::getInstance().stackingPrepareNewEvent();
}

}
