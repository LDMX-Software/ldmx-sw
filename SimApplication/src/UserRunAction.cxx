#include "SimApplication/UserRunAction.h"

// LDMX
#include "Event/RootEventWriter.h"
#include "SimPlugins/PluginManager.h"

using event::RootEventWriter;

namespace sim {

UserRunAction::UserRunAction() {
}

UserRunAction::~UserRunAction() {
}

void UserRunAction::BeginOfRunAction(const G4Run* aRun) {

    std::cout << ">>> Begin Run " << aRun->GetRunID() << " <<<" << std::endl;

    RootEventWriter::getInstance()->open();

    PluginManager::getInstance().beginRun(aRun);
}

void UserRunAction::EndOfRunAction(const G4Run* aRun) {
    RootEventWriter::getInstance()->close();

    PluginManager::getInstance().endRun(aRun);

    std::cout << ">>> End Run " << aRun->GetRunID() << " <<<" << std::endl;
}

}
