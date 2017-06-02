#include "SimApplication/RunManager.h"

// LDMX
#include "SimApplication/APrimePhysics.h"
#include "SimApplication/GammaPhysics.h"
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
#include "G4GenericBiasingPhysics.hh"
#include "G4VModularPhysicsList.hh"
#include "G4ParallelWorldPhysics.hh"

namespace ldmx {

    RunManager::RunManager() {
        pluginManager_ = new PluginManager();
        pluginMessenger_ = new PluginMessenger(pluginManager_);
        pwMessenger_ = new ParallelWorldMessenger(this);
    }

    RunManager::~RunManager() {
        delete pluginManager_;
        delete pluginMessenger_;
    }

    void RunManager::InitializePhysics() {

        G4VUserPhysicsList* thePhysicsList = new FTFP_BERT;
        G4VModularPhysicsList* modularPhysicsList = dynamic_cast<G4VModularPhysicsList*>(thePhysicsList);

        if (isPWEnabled_) {
            std::cout << "[ RunManager ]: Parallel worlds physics list has been registered." << std::endl;
            modularPhysicsList->RegisterPhysics(new G4ParallelWorldPhysics("ldmxParallelWorld"));
        }

        modularPhysicsList->RegisterPhysics(new APrimePhysics);
        modularPhysicsList->RegisterPhysics(new GammaPhysics);
        //modularPhysicsList->RegisterPhysics(new TungstenIonPhysics);

        if (BiasingMessenger::isBiasingEnabled()) {

            std::cout << "[ RunManager ]: Enabling biasing of particle type " << BiasingMessenger::getParticleType() << std::endl;

            // Instantiate the constructor used when biasing
            G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();

            // Specify what particles are being biased
            biasingPhysics->Bias(BiasingMessenger::getParticleType());

            // Register the physics constructor to the physics list:
            modularPhysicsList->RegisterPhysics(biasingPhysics);
        }

        SetUserInitialization(thePhysicsList);

        G4RunManager::InitializePhysics();
    }

    void RunManager::Initialize() {
        
        
        // The parallel world needs to be registered before the mass world is
        // constructed i.e. before G4RunManager::Initialize() is called. 
        if (isPWEnabled_) {
            std::cout << "[ RunManager ]: Parallel worlds have been enabled." << std::endl;

            G4GDMLParser* pwParser = new G4GDMLParser();
            pwParser->Read(parallelWorldPath_);
            this->getDetectorConstruction()->RegisterParallelWorld(new ParallelWorld(pwParser, "ldmxParallelWorld"));
        }

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
