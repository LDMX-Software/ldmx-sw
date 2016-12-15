#ifndef SIMAPPLICATION_ECALSD_H_
#define SIMAPPLICATION_ECALSD_H_

// LDMX
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "SimApplication/CalorimeterSD.h"

// ROOT
#include "TH2Poly.h"
#include "TMath.h"

// Geant4
#include "G4Polyhedra.hh"

using detdescr::EcalDetectorID;
using detdescr::DetectorID;
using detdescr::EcalHexReadout;

namespace sim {

class EcalSD : public CalorimeterSD {

    public:
        EcalSD(G4String name, G4String theCollectionName, int subdet, DetectorID* detID = new EcalDetectorID);

        virtual ~EcalSD();

        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

    private:

        /**
         * Return the hit position of a step.
         * X and Y are computed from the midpoint of the step.
         * Z corresponds to the volume's center.
         */
        G4ThreeVector getHitPosition(G4Step*);

    private:
        EcalHexReadout* hitMap_;
        std::map<G4VSolid*, G4Polyhedron*> polyMap_;
};

}

#endif
