#ifndef SimApplication_TrackerSD_h
#define SimApplication_TrackerSD_h

// Geant4
#include <DetDescr/DetectorID.h>
#include "G4VSensitiveDetector.hh"

// LDMX
#include "SimApplication/G4TrackerHit.h"
#include "Event/Event.h"

class TrackerSD: public G4VSensitiveDetector {

    public:

        TrackerSD(G4String name, G4String theCollectionName, int subdetId, DetectorID* detId);

        virtual ~TrackerSD();

        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

        void Initialize(G4HCofThisEvent* hcEvent);

        void EndOfEvent(G4HCofThisEvent* hcEvent);

    private:

        G4TrackerHitsCollection* hitsCollection;
        Event* currentEvent;
        int subdetId;
        DetectorID* detId;
};

#endif
