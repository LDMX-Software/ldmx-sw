
#include "SimApplication/ParallelWorld.h"

ldmx::ParallelWorld::ParallelWorld(G4GDMLParser* parser, G4String worldName) 
    : G4VUserParallelWorld(worldName), parser_(parser), 
      auxInfoReader_(new AuxInfoReader(parser)) {
}

ldmx::ParallelWorld::~ParallelWorld() {
   delete auxInfoReader_;  
}

void ldmx::ParallelWorld::Construct() {
}

void ldmx::ParallelWorld::ConstructSD() {

    G4VPhysicalVolume* worldPhysical = GetWorld(); 
    G4LogicalVolume* worldLogical = worldPhysical->GetLogicalVolume();
    
    G4LogicalVolume* parallelWorldLogical = parser_->GetWorldVolume()->GetLogicalVolume();
    auxInfoReader_->readGlobalAuxInfo();
     
    for (int index = 0; index < parallelWorldLogical->GetNoDaughters(); index++) { 
        G4VPhysicalVolume* physicalVol = parallelWorldLogical->GetDaughter(index); 
        std::cout << "[ ParallelWorld ]: Adding : " 
                  << physicalVol->GetName() << " to parallel world." << std::endl;
        worldLogical->AddDaughter(physicalVol);
    }

    auxInfoReader_->assignAuxInfoToVolumes();
}
