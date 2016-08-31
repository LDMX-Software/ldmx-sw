#ifndef SIMAPPLICATION_DETECTORCONSTRUCTION_H_
#define SIMAPPLICATION_DETECTORCONSTRUCTION_H_

#include "G4VUserDetectorConstruction.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {

    public:

        DetectorConstruction(G4VPhysicalVolume* world = 0) {
            _world = world;
        }

        virtual G4VPhysicalVolume *Construct() {
            return _world;
        }

    private:
        G4VPhysicalVolume* _world;
};

#endif
