
#include "SimCore/ParallelWorld.h"

#include "Framework/Configure/Parameters.h"

namespace simcore {
ParallelWorld::ParallelWorld(G4GDMLParser* parser, G4String worldName)
    : G4VUserParallelWorld(worldName),
      parser_(parser),
      aux_info_reader_(
          new geo::AuxInfoReader(parser, framework::config::Parameters())) {}

ParallelWorld::~ParallelWorld() { delete aux_info_reader_; }

void ParallelWorld::Construct() {}

void ParallelWorld::ConstructSD() {
  G4VPhysicalVolume* world_physical = GetWorld();
  G4LogicalVolume* world_logical = world_physical->GetLogicalVolume();

  G4LogicalVolume* parallel_world_logical =
      parser_->GetWorldVolume()->GetLogicalVolume();
  aux_info_reader_->readGlobalAuxInfo();

  for (int index = 0; index < parallel_world_logical->GetNoDaughters();
       index++) {
    G4VPhysicalVolume* physical_vol =
        parallel_world_logical->GetDaughter(index);
    ldmx_log(debug) << "Adding : " << physical_vol->GetName()
                    << " to parallel world.";
    world_logical->AddDaughter(physical_vol);
  }

  aux_info_reader_->assignAuxInfoToVolumes();
}
}  // namespace simcore
