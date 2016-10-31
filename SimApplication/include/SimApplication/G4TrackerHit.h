#ifndef SIMAPPLICATION_G4SIMTRACKERHIT_H_
#define SIMAPPLICATION_G4SIMTRACKERHIT_H_

// Geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// LDMX
#include "Event/SimTrackerHit.h"

using event::SimTrackerHit;

namespace sim {

class G4TrackerHit: public G4VHit {

    public:

        G4TrackerHit() {;}

        virtual ~G4TrackerHit() {;}

        void Draw();

        void Print() {;}

        inline void *operator new(size_t);

        inline void operator delete(void *aHit);

        void setTrackID(int trackID) {
            this->trackID = trackID;
        }

        int getTrackID() {
            return this->trackID;
        }

        void setID(int id) {
            this->id = id;
        }

        void setLayerID(int layerID) {
            this->layerID = layerID;
        }

        void setEdep(float edep) {
            this->edep = edep;
        }

        void setTime(float time) {
            this->time = time;
        }

        void setMomentum(float px, float py, float pz) {
            momentum.setX(px);
            momentum.setY(py);
            momentum.setZ(pz);
        }

        void setPosition(float x, float y, float z) {
            position.setX(x);
            position.setY(y);
            position.setZ(z);
        }

        void setPathLength(float pathLength) {
            this->pathLength = pathLength;
        }

        /**
         * Copy the contents of this hit into an output SimTrackerHit.
         */
        void setSimTrackerHit(SimTrackerHit* simTrackerHit) {

            simTrackerHit->setID(id);
            simTrackerHit->setLayerID(layerID);
            simTrackerHit->setEdep(edep);
            simTrackerHit->setTime(time);
            simTrackerHit->setMomentum(
                    momentum.x(),
                    momentum.y(),
                    momentum.z());
            simTrackerHit->setPosition(
                    position.x(),
                    position.y(),
                    position.z());
            simTrackerHit->setPathLength(pathLength);

            this->simTrackerHit = simTrackerHit;
        }

        SimTrackerHit* getSimTrackerHit() {
            return simTrackerHit;
        }

    private:

        G4int trackID{0};
        int id{0};
        int layerID{0};
        float edep{0};
        float time{0};
        G4ThreeVector momentum;
        G4ThreeVector position;
        float pathLength{0};

        SimTrackerHit* simTrackerHit{nullptr};
};

/**
 * Template instantiation of G4 hits collection class.
 */
typedef G4THitsCollection<G4TrackerHit> G4TrackerHitsCollection;

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

}

#endif
