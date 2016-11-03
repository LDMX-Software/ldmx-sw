#include "SimApplication/UserEventAction.h"

// LDMX
#include "Event/RootEventWriter.h"
#include "SimApplication/RootPersistencyManager.h"
#include "SimApplication/TrackMap.h"
#include "SimApplication/TrajectoryContainer.h"
#include "SimPlugins/PluginManager.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Run.hh"

// STL
#include <iostream>
#include <stdio.h>
#include <time.h>

using event::Event;
using event::RootEventWriter;

namespace sim {

void UserEventAction::BeginOfEventAction(const G4Event* anEvent) {

    // Clear the global track map.
    TrackMap::getInstance()->clear();

    // Install custom trajectory container for the event.
    G4EventManager::GetEventManager()->GetNonconstCurrentEvent()->SetTrajectoryContainer(new TrajectoryContainer);

    // Clear the current ROOT event object.
    RootPersistencyManager* rootIO = RootPersistencyManager::getInstance();
    if (rootIO != nullptr) {
        rootIO->clearCurrentEvent();
    }

    PluginManager::getInstance().beginEvent(anEvent);
}

void UserEventAction::EndOfEventAction(const G4Event* anEvent) {

    PluginManager::getInstance().endEvent(anEvent);

    std::cout << ">>> End Event " << anEvent->GetEventID() << " <<<" << std::endl;
}

}
