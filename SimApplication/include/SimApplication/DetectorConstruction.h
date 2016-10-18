#ifndef SimApplication_DetectorConstruction_h
#define SimApplication_DetectorConstruction_h

// LDMX
#include "AuxInfoReader.h"

// Geant4
#include "G4VUserDetectorConstruction.hh"
#include "G4GDMLParser.hh"

namespace sim {

class DetectorConstruction: public G4VUserDetectorConstruction {

    public:

        DetectorConstruction(G4GDMLParser* theParser);

        virtual ~DetectorConstruction();

        G4VPhysicalVolume *Construct();

    private:
        G4GDMLParser* parser;
        AuxInfoReader* auxInfoReader;
};

}

#endif
