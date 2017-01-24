#include "SimApplication/DetectorConstruction.h"

namespace ldmx {

DetectorConstruction::DetectorConstruction(G4GDMLParser* theParser) :
    parser_(theParser),
    auxInfoReader_(new AuxInfoReader(theParser)) {
    }

DetectorConstruction::~DetectorConstruction() {
    delete auxInfoReader_;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auxInfoReader_->readGlobalAuxInfo();
    auxInfoReader_->assignAuxInfoToVolumes();
    return parser_->GetWorldVolume();
}

void DetectorConstruction::ConstructSDandField() {

    // Instantiate the biasing operator
    PhotonuclearXsecBiasingOperator* xsecBiasing 
        = new PhotonuclearXsecBiasingOperator("PhotonuclearXsecBiasingOperator");

    // TODO: This should be configurable from a macro
    G4LogicalVolume* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("target");
    xsecBiasing->AttachTo(logicalVolume);
    /*G4LogicalVolume* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("em_calorimeters");
    for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) { 
        G4String volumeName = volume->GetName();
        std::cout << "Volume name: " << volumeName << std::endl;
        if (volumeName.contains("W") && volumeName.contains("log")) { 
            xsecBiasing->AttachTo(volume);
            G4cout << " Attaching biasing operator " << xsecBiasing->GetName()
                   << " to logical volume " << logicalVolume->GetName()
                   << G4endl;
        }
    }*/
}

}
