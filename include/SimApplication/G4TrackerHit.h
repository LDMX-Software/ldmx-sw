#ifndef SIMAPPLICATION_G4SIMTRACKER_HIT_H
#define SIMAPPLICATION_G4SIMTRACKER_HIT_H 1

// Geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// LDMX
#include "Event/SimTrackerHit.h"

class G4TrackerHit : G4VHit {

public:

    G4TrackerHit();

    virtual ~G4TrackerHit();

    void Draw();

    void Print();

    inline void *operator new(size_t);

    inline void operator delete(void *aHit);

    SimTrackerHit* getSimTrackerHit();

    G4ThreeVector getPosition();

    void setStartPosition(const G4ThreeVector& startPosition);

    void setEndPosition(const G4ThreeVector& endPosition);

    void setMomentum(const G4ThreeVector& endPosition);

private:

    SimTrackerHit* simTrackerHit;
};

/**
 * Template instantiation of G4 hits collection class.
 */
typedef G4THitsCollection<G4TrackerHit> (TrackerHitsCollection);

/**
 * Memory allocator for objects of this class.
 */
extern G4Allocator<G4TrackerHit> G4TrackerHitAllocator;

/**
 * Implementation of custom new operator.
 */
inline void* G4TrackerHit::operator new(size_t) {
    void* aHit;
    aHit = (void*) G4TrackerHitAllocator.MallocSingle();
    return aHit;
}

/**
 * Implementation of custom delete operator.
 */
inline void G4TrackerHit::operator delete(void *aHit) {
    G4TrackerHitAllocator.FreeSingle((G4TrackerHit*) aHit);
}

#endif
