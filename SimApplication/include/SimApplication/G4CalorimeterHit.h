#ifndef SimApplication_G4CalorimeterHit_h
#define SimApplication_G4CalorimeterHit_h

// Geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// LDMX
#include "Event/SimCalorimeterHit.h"

using event::SimCalorimeterHit;

namespace sim {

class G4CalorimeterHit: public G4VHit {

    public:

        G4CalorimeterHit() {;}

        virtual ~G4CalorimeterHit() {;}

        void Draw();

        void Print() {;}

        inline void *operator new(size_t);

        inline void operator delete(void *aHit);

        G4int getTrackID() {
            return trackID;
        }

        void setTrackID(int trackID) {
            this->trackID = trackID;
        }

        void setID(int id) {
            this->id = id;
        }

        void setEdep(float edep) {
            this->edep = edep;
        }

        void setPosition(const float x, const float y, const float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        void setTime(float time) {
            this->time = time;
        }

        void setSimCalorimeterHit(SimCalorimeterHit* simCalHit) {
            simCalHit->setID(id);
            simCalHit->setEdep(edep);
            simCalHit->setPosition(x, y, z);
            simCalHit->setTime(time);
            this->simCalHit = simCalHit;
        }

        SimCalorimeterHit* getSimCalorimeterHit() {
            return simCalHit;
        }

    private:

        int trackID{-1};
        int id{0};
        double edep{0};
        double x{0};
        double y{0};
        double z{0};
        float time{0};

        SimCalorimeterHit* simCalHit{nullptr};
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

}

#endif
