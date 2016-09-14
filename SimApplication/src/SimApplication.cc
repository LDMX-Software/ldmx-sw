// LDMX
#include "SimApplication/SimApplication.h"
#include "SimApplication/PrimaryGeneratorAction.h"

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

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

SimApplication::SimApplication() :
    _messenger(0) {
}

SimApplication::~SimApplication() {
    delete _messenger;
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

    // Create application messenger which defines macro commands.
    _messenger = new SimApplicationMessenger();

    // Default user actions.
    runManager->SetUserInitialization(new FTFP_BERT);
    runManager->SetUserAction(new PrimaryGeneratorAction());

    // Initialize G4 visualization framework.
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // Get the pointer to the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    if (ui == 0) {
        // execute macro provided on command line
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        std::cout << "executing macro " << fileName << " ..." << std::endl;
        UImanager->ApplyCommand(command + fileName);
    } else {
        // start an interactive session
        std::cout << "starting interactive session ..." << std::endl;
        ui->SessionStart();
        delete ui;
    }

    // print out aux vol info
    /*
    const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
    std::vector<G4LogicalVolume*>::const_iterator lvciter;
    for (lvciter = lvs->begin(); lvciter != lvs->end(); lvciter++) {
        G4GDMLAuxListType aux_info = parser.GetVolumeAuxiliaryInformation(*lvciter);
        std::vector<G4GDMLAuxStructType>::const_iterator ipair = auxInfo.begin();
        for (ipair = aux_info.begin(); ipair != aux_info.end(); ipair++) {
            G4String str = ipair->type;
            G4String val = ipair->value;
            G4cout << " Auxiliary Information is found for Logical Volume :  " << (*lvciter)->GetName() << G4endl;
            G4cout << " Name of Auxiliary type is     :  " << str << G4endl;
            G4cout << " Associated Auxiliary value is :  " << val << G4endl; 
        } 
    }
    */

    delete runManager;
}
