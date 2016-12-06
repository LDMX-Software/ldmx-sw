#include "SimApplication/RunManager.h"

// LDMX
#include "SimApplication/APrimePhysics.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"
#include "SimApplication/RootPersistencyMessenger.h"
#include "SimApplication/SteppingAction.h"
#include "SimApplication/UserEventAction.h"
#include "SimApplication/UserRunAction.h"
#include "SimApplication/UserStackingAction.h"
#include "SimApplication/UserTrackingAction.h"

// Geant4
#include "FTFP_BERT.hh"
#include "G4VModularPhysicsList.hh"

namespace sim {

RunManager::RunManager() {
    pluginManager_ = new PluginManager();
    pluginMessenger_ = new PluginMessenger(pluginManager_);
}

RunManager::~RunManager() {
    delete pluginManager_;
    delete pluginMessenger_;
}

void RunManager::InitializePhysics() {

    G4VUserPhysicsList* thePhysicsList = new FTFP_BERT;
    G4VModularPhysicsList* modularPhysicsList = dynamic_cast<G4VModularPhysicsList*>(thePhysicsList);

    modularPhysicsList->RegisterPhysics(new APrimePhysics);
    //modularPhysicsList->RegisterPhysics(new TungstenIonPhysics);

    SetUserInitialization(thePhysicsList);

    G4RunManager::InitializePhysics();
}

void RunManager::Initialize() {

    G4RunManager::Initialize();

    PrimaryGeneratorAction* primaryGeneratorAction = new PrimaryGeneratorAction;
    SetUserAction(primaryGeneratorAction);
    new PrimaryGeneratorMessenger(primaryGeneratorAction);

    UserRunAction* runAction = new UserRunAction;
    UserEventAction* eventAction = new UserEventAction;
    UserTrackingAction* trackingAction = new UserTrackingAction;
    SteppingAction* steppingAction = new SteppingAction;
    UserStackingAction* stackingAction = new UserStackingAction;

    runAction->setPluginManager(pluginManager_);
    eventAction->setPluginManager(pluginManager_);
    trackingAction->setPluginManager(pluginManager_);
    steppingAction->setPluginManager(pluginManager_);
    stackingAction->setPluginManager(pluginManager_);
    primaryGeneratorAction->setPluginManager(pluginManager_);

    SetUserAction(runAction);
    SetUserAction(eventAction);
    SetUserAction(trackingAction);
    SetUserAction(steppingAction);
    SetUserAction(stackingAction);

    RootPersistencyManager* rootIO = new RootPersistencyManager();
    new RootPersistencyMessenger(rootIO);
}

}
