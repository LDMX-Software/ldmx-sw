// LDMX
#include "SimApplication/SimApplication.h"
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/PrimaryGeneratorAction.h"

// STL
#include <stdio.h>
#include <vector>

// Geant4
#include "G4RunManager.hh"
#include "G4GDMLParser.hh"
#include "FTFP_BERT.hh"
#include "G4LogicalVolumeStore.hh"


void SimApplication::run(const char* argv[]) {

    // load GDML file (hard-coded path for now)
    G4GDMLParser parser;
    parser.Read("geom.gdml");

    // run manager init
    G4RunManager* run_manager = new G4RunManager;
    run_manager->SetUserInitialization(new DetectorConstruction(parser.GetWorldVolume()));
    run_manager->SetUserInitialization(new FTFP_BERT);
    run_manager->SetUserAction(new PrimaryGeneratorAction());
    run_manager->Initialize();

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

    delete run_manager;
}    
