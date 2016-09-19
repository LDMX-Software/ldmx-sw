#ifndef CALORIMETERSD_SIMAPPLICATION_H_
#define CALORIMETERSD_SIMAPPLICATION_H_ 1

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "SimApplication/G4CalorimeterHit.h"
#include "Event/Event.h"

class CalorimeterSD: public G4VSensitiveDetector {

    public:

        CalorimeterSD(G4String name, G4String theCollectionName, int subdetId);

        virtual ~CalorimeterSD();

        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

        void Initialize(G4HCofThisEvent* hcEvent);

        void EndOfEvent(G4HCofThisEvent* hcEvent);

    private:

        G4CalorimeterHitsCollection* hitsCollection;
        Event* currentEvent;
        int subdetId;
};

#endif
