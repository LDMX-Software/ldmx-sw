#ifndef SIMAPPLICATION_DETECTORCONSTRUCTION_H_
#define SIMAPPLICATION_DETECTORCONSTRUCTION_H_

// LDMX
#include "AuxInfoReader.h"

// Biasing
#include "Biasing/PhotonuclearXsecBiasingOperator.h"

// Geant4
#include "G4VUserDetectorConstruction.hh"
#include "G4GDMLParser.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"

namespace sim {

class DetectorConstruction: public G4VUserDetectorConstruction {

    public:

        DetectorConstruction(G4GDMLParser* theParser);

        virtual ~DetectorConstruction();

        G4VPhysicalVolume *Construct();

        void ConstructSDandField();

    private:
        G4GDMLParser* parser_;
        AuxInfoReader* auxInfoReader_;
};

}

#endif
