#ifndef SIMAPPLICATION_CALORIMETERSD_H_
#define SIMAPPLICATION_CALORIMETERSD_H_

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "Event/Event.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "DetDescr/DetectorID.h"

using detdescr::DetectorID;
using event::Event;

namespace sim {

class CalorimeterSD: public G4VSensitiveDetector {

    public:

        CalorimeterSD(G4String name,
                G4String theCollectionName,
                int subdet,
                DetectorID* detID);

        virtual ~CalorimeterSD();

        void setLayerDepth(int layerDepth) {
            this->layerDepth_ = layerDepth;
        }

        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) ;

        void Initialize(G4HCofThisEvent* hcEvent);

        void EndOfEvent(G4HCofThisEvent* hcEvent);

    protected:

        G4CalorimeterHitsCollection* hitsCollection_;
        int subdet_;
        DetectorID* detID_;
        int layerDepth_{2};
};

}

#endif
