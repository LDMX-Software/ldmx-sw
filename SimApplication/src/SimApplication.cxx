#include "SimApplication/SimApplication.h"

// LDMX
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RunManager.h"
#include "SimApplication/SimApplicationMessenger.h"
#include "SimApplication/GDMLMessenger.h"

// STL
#include <vector>
#include <iostream>

// Geant4
#include "G4RunManager.hh"
#include "G4GDMLParser.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4RunManager.hh"

namespace ldmx {

    SimApplication::SimApplication() {
    }

    SimApplication::~SimApplication() {
    }

    void SimApplication::run(int argc, char** argv) {

        std::cout << "[ SimApplication ] : starting" << std::endl;

        // If no arguments then start an interactive session.
        G4UIExecutive* ui = 0;
        if (argc == 1) {
            ui = new G4UIExecutive(argc, argv);
        }

        // Create run manager.
        G4RunManager* runManager = new RunManager;

        // Setup GDML parser and messenger.
        G4GDMLParser* parser = new G4GDMLParser();
        parser->SetAddPointerToName(false);
        parser->SetStripFlag(true);
        G4UImessenger* g4gdmlMessenger = new G4GDMLMessenger(parser);

        // Custom GDML messenger for exporting without pointers appended to names.
        G4UImessenger* gdmlMessenger = new GDMLMessenger(parser);

        // Create application messenger.
        new SimApplicationMessenger();

        // Supply default user initializations and actions.
        runManager->SetUserInitialization(new DetectorConstruction(parser));
        runManager->SetRandomNumberStore(true);

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

        delete g4gdmlMessenger;
        delete gdmlMessenger;
        delete parser;
        delete runManager;

        std::cout << "[ SimApplication ] : exiting" << std::endl;
    }

}
