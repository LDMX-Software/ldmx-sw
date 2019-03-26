/**
 * @file RunManager.cxx
 * @brief Class providing a Geant4 run manager implementation.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/RunManager.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/APrimePhysics.h"
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/GammaPhysics.h"
#include "SimApplication/ParallelWorld.h"
#include "SimApplication/ParallelWorldMessenger.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"
#include "SimApplication/RootPersistencyMessenger.h"
#include "SimApplication/RootPersistencyManager.h" 
#include "SimApplication/SteppingAction.h"
#include "SimApplication/UserEventAction.h"
#include "SimApplication/UserRunAction.h"
#include "SimApplication/UserStackingAction.h"
#include "SimApplication/UserTrackingAction.h"
#include "SimPlugins/PluginManager.h"
#include "SimPlugins/PluginMessenger.h"

//------------//
//   Geant4   //
//------------//
#include "FTFP_BERT.hh"
#include "G4GDMLParser.hh"
#include "G4GenericBiasingPhysics.hh"
#include "G4VModularPhysicsList.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4PhysListFactory.hh"

namespace ldmx {

    RunManager::RunManager() {
        pluginManager_ = new PluginManager();
        pluginMessenger_ = new PluginMessenger(pluginManager_);
        pwMessenger_ = new ParallelWorldMessenger(this);
        
        // Setup messenger for physics list.
        physicsListFactory_ = new G4PhysListFactory;
    }

    RunManager::~RunManager() {
        delete pluginManager_;
        delete pluginMessenger_;
        delete physicsListFactory_; 
    }

    void RunManager::setupPhysics() {

        G4VModularPhysicsList* pList = physicsListFactory_->GetReferencePhysList("FTFP_BERT");
        
        if (isPWEnabled_) {
            std::cout << "[ RunManager ]: Parallel worlds physics list has been registered." << std::endl;
            pList->RegisterPhysics(new G4ParallelWorldPhysics("ldmxParallelWorld"));
        }
        
        pList->RegisterPhysics(new APrimePhysics);
        pList->RegisterPhysics(new GammaPhysics);
       
        if (BiasingMessenger::isBiasingEnabled()) {

            std::cout << "[ RunManager ]: Enabling biasing of particle type " << BiasingMessenger::getParticleType() << std::endl;

            // Instantiate the constructor used when biasing
            G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();

            // Specify what particles are being biased
            biasingPhysics->Bias(BiasingMessenger::getParticleType());

            // Register the physics constructor to the physics list:
            pList->RegisterPhysics(biasingPhysics);
        }

        this->SetUserInitialization(pList);
    }

    void RunManager::Initialize() {
        
        setupPhysics();

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

    DetectorConstruction* RunManager::getDetectorConstruction() {
        return static_cast<DetectorConstruction*>(this->userDetector); 
    }

} // ldmx 
