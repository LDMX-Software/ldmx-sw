#ifndef SimApplication_G4CalorimeterHit_h
#define SimApplication_G4CalorimeterHit_h

// Geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// LDMX
#include "Event/SimCalorimeterHit.h"

class G4CalorimeterHit: public G4VHit {

    public:

        G4CalorimeterHit(SimCalorimeterHit*);

        G4CalorimeterHit();

        virtual ~G4CalorimeterHit();

        void Draw();

        void Print();

        inline void *operator new(size_t);

        inline void operator delete(void *aHit);

        SimCalorimeterHit* getSimCalorimeterHit();

        void setTrackID(G4int trackID);

        G4int getTrackID();

    private:

        SimCalorimeterHit* simCalorimeterHit;
        G4int trackID;
};

/**
 * Template instantiation of G4 hits collection class.
 */
typedef G4THitsCollection<G4CalorimeterHit> G4CalorimeterHitsCollection;

/**
 * Memory allocator for objects of this class.
 */
extern G4Allocator<G4CalorimeterHit> G4CalorimeterHitAllocator;

/**
 * Implementation of custom new operator.
 */
inline void* G4CalorimeterHit::operator new(size_t) {
    void* aHit;
    aHit = (void*) G4CalorimeterHitAllocator.MallocSingle();
    return aHit;
}

/**
 * Implementation of custom delete operator.
 */
inline void G4CalorimeterHit::operator delete(void *aHit) {
    G4CalorimeterHitAllocator.FreeSingle((G4CalorimeterHit*) aHit);
}

#endif
