#include "SimApplication/UserRunAction.h"

// LDMX
#include "SimPlugins/PluginManager.h"
#include "SimApplication/RootPersistencyManager.h"

namespace ldmx {

    UserRunAction::UserRunAction() {
    }

    UserRunAction::~UserRunAction() {
    }

    void UserRunAction::BeginOfRunAction(const G4Run* aRun) {

        // Open the ROOT writer.
        //if (RootPersistencyManager::getInstance()) {
        //    RootPersistencyManager::getInstance()->Initialize();
        //}

        pluginManager_->beginRun(aRun);

    }

    void UserRunAction::EndOfRunAction(const G4Run* aRun) {

        pluginManager_->endRun(aRun);
    }

}
