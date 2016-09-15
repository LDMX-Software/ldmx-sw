#include "SimApplication/DetectorConstruction.h"

DetectorConstruction::DetectorConstruction(G4GDMLParser* theParser) :
    parser(theParser),
    auxInfoReader(new AuxInfoReader(theParser)) {
}

DetectorConstruction::~DetectorConstruction() {
    delete auxInfoReader;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auxInfoReader->readGlobalAuxInfo();
    auxInfoReader->assignSensDetsToVols();
    return parser->GetWorldVolume();
}
