#ifndef SIMAPPLICATION_G4CALORIMETERHIT_H_
#define SIMAPPLICATION_G4CALORIMETERHIT_H_

// Geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

// LDMX
#include "Event/SimCalorimeterHit.h"
#include "Event/ReadoutCalorimeterHit.h"

using event::SimCalorimeterHit;
using event::ReadoutCalorimeterHit;

namespace sim {

class G4CalorimeterHit: public G4VHit {

    public:

        G4CalorimeterHit() {;}

        virtual ~G4CalorimeterHit() {;}

        void Draw();

        void Print();

        std::ostream& print(std::ostream& os);

        inline void *operator new(size_t);

        inline void operator delete(void *aHit);

        G4int getTrackID() {
            return trackID_;
        }

        void setTrackID(int trackID) {
            this->trackID_ = trackID;
        }

        void setID(int id) {
            this->id_ = id;
        }

        int getID(){
        	return id_;
        }

        void setEdep(float edep) {
            this->edep_ = edep;
        }

        float getEdep() {
            return edep_;
        }

        void setPosition(const float x, const float y, const float z) {
            position_.set(x, y, z);
        }

        void setTime(float time) {
            this->time_ = time;
        }

        void setSimCalorimeterHit(SimCalorimeterHit* simCalHit) {
            simCalHit->setID(id_);
            simCalHit->setEdep(edep_);
            simCalHit->setPosition(position_.x(), position_.y(), position_.z());
            simCalHit->setTime(time_);
            this->simCalHit_ = simCalHit;
        }

        void ReadCalorimeterHit(ReadoutCalorimeterHit* readoutHit, bool existingHit) {

			if(existingHit){
				readoutHit->setEdep(edep_ + readoutHit->getEdep());
			}
			else{
				readoutHit->setID(id_);
				readoutHit->setEdep(edep_);
				readoutHit->setPosition(position_.x(), position_.y(), position_.z());
				readoutHit->setTime(time_);
				this->readCalHit_ = readoutHit;
			}


        }

        SimCalorimeterHit* getSimCalorimeterHit() {
            return simCalHit_;
        }
        ReadoutCalorimeterHit* getReadCalorimeterHit() {
            return readCalHit_;
        }
    private:

        int trackID_{-1};
        int id_{0};
        double edep_{0};
        G4ThreeVector position_;
        float time_{0};

        SimCalorimeterHit* simCalHit_{nullptr};
        ReadoutCalorimeterHit* readCalHit_{nullptr};

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
