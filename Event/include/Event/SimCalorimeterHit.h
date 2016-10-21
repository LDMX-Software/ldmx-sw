#ifndef Event_SimCalorimeterHit_h
#define Event_SimCalorimeterHit_h

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

namespace event {

class SimCalorimeterHit: public TObject {

    public:

        SimCalorimeterHit() {;}

        virtual ~SimCalorimeterHit() {;}

        void Print(Option_t *option = "") const;

        int getID() {
            return id;
        }

        float getEdep() {
            return edep;
        }

        std::vector<float> getPosition() const {
            return {x, y, z};
        }

        float getTime() {
            return time;
        }

        SimParticle* getSimParticle() {
            return (SimParticle*) simParticle.GetObject();
        }

        void setID(const int id) {
            this->id = id;
        }

        void setEdep(const float edep) {
            this->edep = edep;
        }

        void setPosition(const float x, const float y, const float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        void setTime(const float time) {
            this->time = time;
        }

        void setSimParticle(SimParticle* simParticle) {
            this->simParticle = simParticle;
        }

    private:

        int id{0};
        float edep{0};
        float x{0};
        float y{0};
        float z{0};
        float time{0};

        TRef simParticle{nullptr};

    ClassDef(SimCalorimeterHit, 1)
};

}

#endif
