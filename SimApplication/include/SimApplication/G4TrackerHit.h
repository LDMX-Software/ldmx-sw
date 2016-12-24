#ifndef SIMAPPLICATION_G4SIMTRACKERHIT_H_
#define SIMAPPLICATION_G4SIMTRACKERHIT_H_

// Geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// LDMX
#include "Event/SimTrackerHit.h"

// STL
#include <ostream>
#include <vector>

using event::SimTrackerHit;
using std::vector;

namespace sim {

class G4TrackerHit: public G4VHit {

    public:

        G4TrackerHit() {;}

        virtual ~G4TrackerHit() {;}

        void Draw();

        void Print();

        std::ostream& print(std::ostream& os);

        inline void *operator new(size_t);

        inline void operator delete(void *aHit);

        void setTrackID(int trackID) {
            this->trackID_ = trackID;
        }

        int getTrackID() {
            return this->trackID_;
        }

        int getID() {
            return id_;
        }

        void setID(int id) {
            this->id_ = id;
        }

        int getLayerID() {
            return layerID_;
        }

        void setLayerID(int layerID) {
            this->layerID_ = layerID;
        }

        float getEdep() {
            return edep_;
        }

        void setEdep(float edep) {
            this->edep_ = edep;
        }

        float getTime() {
            return time_;
        }

        void setTime(float time) {
            this->time_ = time;
        }

        const G4ThreeVector& getMomentum() {
            return momentum_;
        }

        void setMomentum(float px, float py, float pz) {
            momentum_.setX(px);
            momentum_.setY(py);
            momentum_.setZ(pz);
        }

        const G4ThreeVector& getPosition() {
            return position_;
        }

        void setPosition(float x, float y, float z) {
            position_.setX(x);
            position_.setY(y);
            position_.setZ(z);
        }

        float getPathLength() {
            return pathLength_;
        }

        void setPathLength(float pathLength) {
            this->pathLength_ = pathLength;
        }

        SimTrackerHit* getSimTrackerHit() {
            return simTrackerHit_;
        }

    private:

        G4int trackID_{0};
        int id_{0};
        int layerID_{0};
        float edep_{0};
        float time_{0};
        G4ThreeVector momentum_;
        G4ThreeVector position_;
        float pathLength_{0};

        SimTrackerHit* simTrackerHit_{nullptr};
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
