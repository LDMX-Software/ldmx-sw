#ifndef TRACKERSD_SIMAPPLICATION_H_
#define TRACKERSD_SIMAPPLICATION_H_ 1

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "SimApplication/G4TrackerHit.h"
#include "Event/Event.h"

class TrackerSD: public G4VSensitiveDetector {

    public:

        TrackerSD(G4String name, G4String theCollectionName, int subdetId);

        virtual ~TrackerSD();

        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

        void Initialize(G4HCofThisEvent* hcEvent);

        void EndOfEvent(G4HCofThisEvent* hcEvent);

    private:

        G4TrackerHitsCollection* hitsCollection;
        Event* currentEvent;
        int subdetId;

};

#endif
