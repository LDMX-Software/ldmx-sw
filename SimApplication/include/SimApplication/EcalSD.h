#ifndef SIMAPPLICATION_ECALSD_H_
#define SIMAPPLICATION_ECALSD_H_

// LDMX
#include "Event/Event.h"
#include "SimApplication/CalorimeterSD.h"

// ROOT
#include "TH2Poly.h"
#include "TMath.h"
using detdescr::DetectorID;
using event::Event;

namespace sim {

class EcalSD : public CalorimeterSD {

    public:
        enum {EcalDetectorID = 5} ;
        EcalSD(G4String name, G4String theCollectionName, int subdet = EcalDetectorID,
                DetectorID* detID);

        virtual ~EcalSD();

        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

    private:
        TH2Poly *map;
};

}

#endif
