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

}
