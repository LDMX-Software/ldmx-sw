#include "SimApplication/UserEventAction.h"

// LDMX
#include "SimApplication/RootPersistencyManager.h"
#include "SimApplication/TrackMap.h"
#include "SimApplication/TrajectoryContainer.h"
#include "SimApplication/UserTrackingAction.h"
#include "SimPlugins/PluginManager.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Run.hh"

// STL
#include <iostream>
#include <stdio.h>
#include <time.h>

namespace ldmx {

void UserEventAction::BeginOfEventAction(const G4Event* anEvent) {

    // Clear the global track map.
    UserTrackingAction::getUserTrackingAction()->getTrackMap()->clear();

    // Install custom trajectory container for the event.
    G4EventManager::GetEventManager()->GetNonconstCurrentEvent()->SetTrajectoryContainer(new TrajectoryContainer);

    // Activate user plugins.
    pluginManager_->beginEvent(anEvent);
}

void UserEventAction::EndOfEventAction(const G4Event* anEvent) {
    pluginManager_->endEvent(anEvent);
}

}
