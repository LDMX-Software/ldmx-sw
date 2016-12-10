#ifndef SIMAPPLICATION_ECALSD_H_
#define SIMAPPLICATION_ECALSD_H_

// LDMX
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "SimApplication/CalorimeterSD.h"

// ROOT
#include "TH2Poly.h"
#include "TMath.h"

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
        EcalHexReadout* hitMap;
};

}

#endif
