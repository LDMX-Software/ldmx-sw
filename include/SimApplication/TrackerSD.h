#ifndef TRACKERSD_SIMAPPLICATION_H_
#define TRACKERSD_SIMAPPLICATION_H_ 1

#include "G4VSensitiveDetector.hh"

class TrackerSD : public G4VSensitiveDetector {

public:

    TrackerSD(G4String name, G4String theCollectionName);

    virtual ~TrackerSD();

    virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
};

#endif
