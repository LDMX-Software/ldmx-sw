#ifndef SimApplication_CalorimeterSD_h
#define SimApplication_CalorimeterSD_h

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "DetDescr/DetectorID.h"
#include "Event/Event.h"
#include "SimApplication/G4CalorimeterHit.h"

using detdescr::DetectorID;
using event::Event;

namespace sim {

class CalorimeterSD: public G4VSensitiveDetector {

    public:

        CalorimeterSD(G4String theName, G4String theCollectionName, int theSubdetId, DetectorID* theDetId);

        virtual ~CalorimeterSD();

        void setLayerDepth(int layerDepth) {
            this->layerDepth = layerDepth;
        }

        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

        void Initialize(G4HCofThisEvent* hcEvent);

        void EndOfEvent(G4HCofThisEvent* hcEvent);

    private:

        G4CalorimeterHitsCollection* hitsCollection;
        int subdetId;
        DetectorID* detId;
        int layerDepth{2};
};

}

#endif
