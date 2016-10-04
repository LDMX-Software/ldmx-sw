#include "SimApplication/SimApplication.h"

// LDMX
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"
#include "SimApplication/SimApplicationMessenger.h"
#include "SimApplication/UserEventAction.h"
#include "SimApplication/UserRunAction.h"
#include "SimApplication/UserTrackingAction.h"


// STL
#include <vector>
#include <iostream>

// Geant4
#include "G4RunManager.hh"
#include "G4GDMLParser.hh"
#include "FTFP_BERT.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4RunManager.hh"

SimApplication::SimApplication() {
}

SimApplication::~SimApplication() {
}

void SimApplication::run(int argc, char** argv) {

    // If no arguments then start an interactive session.
    G4UIExecutive* ui = 0;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    // Create run manager.
#ifdef G4MULTITHREADED
    G4MTRunManager* runManager = new G4MTRunManager;
#else
    G4RunManager* runManager = new G4RunManager;
#endif

    // Setup GDML parser and messenger.
    G4GDMLParser* parser = new G4GDMLParser();
    G4UImessenger* gdmlMessenger = new G4GDMLMessenger(parser);

    // Create application messenger.
    new SimApplicationMessenger();

    // Supply default user initializations and actions.
    runManager->SetUserInitialization(new FTFP_BERT);
    runManager->SetUserInitialization(new DetectorConstruction(parser));
    PrimaryGeneratorAction* pga = new PrimaryGeneratorAction;
    runManager->SetUserAction(pga);
    runManager->SetUserAction(new UserEventAction);
    runManager->SetUserAction(new UserRunAction);
    runManager->SetUserAction(new UserTrackingAction);

    // Create primary generator messenger.
    new PrimaryGeneratorMessenger(pga);

    // Initialize G4 visualization framework.
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // Get the pointer to the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    if (ui == 0) {
        // execute macro provided on command line
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        std::cout << "Executing macro " << fileName << " ..." << std::endl;
        UImanager->ApplyCommand(command + fileName);
    } else {
        // start an interactive session
        std::cout << "Starting interactive session ..." << std::endl;
        ui->SessionStart();
        delete ui;
    }

    delete gdmlMessenger;
    delete parser;
    delete runManager;
}
