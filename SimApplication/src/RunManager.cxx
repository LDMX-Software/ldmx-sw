#include "SimApplication/RunManager.h"

// LDMX
#include "SimApplication/APrimePhysics.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"
#include "SimApplication/SteppingAction.h"
#include "SimApplication/UserEventAction.h"
#include "SimApplication/UserRunAction.h"
#include "SimApplication/UserTrackingAction.h"

// Geant4
#include "FTFP_BERT.hh"
#include "G4VModularPhysicsList.hh"

namespace sim {

RunManager::RunManager() {
    pluginManager = &PluginManager::getInstance();
    pluginMessenger = new PluginMessenger(pluginManager);
}

RunManager::~RunManager() {
    delete pluginManager;
    delete pluginMessenger;
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

    PrimaryGeneratorAction* pga = new PrimaryGeneratorAction;
    SetUserAction(pga);
    new PrimaryGeneratorMessenger(pga);
    SetUserAction(new UserRunAction);
    SetUserAction(new UserEventAction);
    SetUserAction(new UserTrackingAction);
    SetUserAction(new SteppingAction);
}

}
