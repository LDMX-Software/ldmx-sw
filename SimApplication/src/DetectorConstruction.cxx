#include "SimApplication/DetectorConstruction.h"

namespace sim {

DetectorConstruction::DetectorConstruction(G4GDMLParser* theParser) :
    parser(theParser),
    auxInfoReader(new AuxInfoReader(theParser)) {
}

DetectorConstruction::~DetectorConstruction() {
    delete auxInfoReader;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auxInfoReader->readGlobalAuxInfo();
    auxInfoReader->assignAuxInfoToVolumes();
    return parser->GetWorldVolume();
}

}
