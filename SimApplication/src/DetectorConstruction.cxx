#include "SimApplication/DetectorConstruction.h"

namespace sim {

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

    // TODO: This should be configurable from a macro
    G4LogicalVolume* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("target");

    // Instantiate the biasing operator
    PhotonuclearXsecBiasingOperator* xsecBiasing 
        = new PhotonuclearXsecBiasingOperator("PhotonuclearXsecBiasingOperator");
    
    // Attach it to the volume that will be biased.
    xsecBiasing->AttachTo(logicalVolume);
    G4cout << " Attaching biasing operator " << xsecBiasing->GetName()
           << " to logical volume " << logicalVolume->GetName()
           << G4endl;
}

}
