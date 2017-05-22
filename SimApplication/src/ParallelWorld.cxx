
#include "SimApplication/ParallelWorld.h"


ldmx::ParallelWorld::ParallelWorld(G4GDMLParser* parser, G4String worldName) 
    : G4VUserParallelWorld(worldName), parser_(parser), 
      auxInfoReader_(new AuxInfoReader(parser)) {
}

ldmx::ParallelWorld::~ParallelWorld() { 
}

void ldmx::ParallelWorld::Construct() {
    auxInfoReader_->readGlobalAuxInfo();
    auxInfoReader_->assignAuxInfoToVolumes();
}

void ldmx::ParallelWorld::ConstructSD() {

    G4VPhysicalVolume* worldPhysical = GetWorld(); 
    G4LogicalVolume* worldLogical = worldPhysical->GetLogicalVolume();
    
    G4LogicalVolume* parallelWorldLogical = parser_->GetWorldVolume()->GetLogicalVolume();
    for (int index = 0; index < parallelWorldLogical->GetNoDaughters(); index++) { 
        std::cout << "[ ParallelWorld ]: Adding : " 
                  << parallelWorldLogical->GetDaughter(index)->GetName() 
                  << " to parallel world." << std::endl;
        worldLogical->AddDaughter(parallelWorldLogical->GetDaughter(index));
    }
}
