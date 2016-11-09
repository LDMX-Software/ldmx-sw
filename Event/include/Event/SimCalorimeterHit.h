#ifndef EVENT_SIMCALORIMETERHIT_H_
#define EVENT_SIMCALORIMETERHIT_H_

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

namespace event {

class SimCalorimeterHit: public TObject {

    public:

        SimCalorimeterHit();

        virtual ~SimCalorimeterHit();

        void Clear(Option_t *option = "");

        void Print(Option_t *option = "") const;

        int getID() {
            return id_;
        }

        float getEdep() {
            return edep_;
        }

        std::vector<float> getPosition() const {
            return {x_, y_, z_};
        }

        float getTime() {
            return time_;
        }

        SimParticle* getSimParticle() {
            return (SimParticle*) simParticle_.GetObject();
        }

        void setID(const int id) {
            this->id_ = id;
        }

        void setEdep(const float edep) {
            this->edep_ = edep;
        }

        void setPosition(const float x, const float y, const float z) {
            this->x_ = x;
            this->y_ = y;
            this->z_ = z;
        }

        void setTime(const float time) {
            this->time_ = time;
        }

        void setSimParticle(SimParticle* simParticle) {
            this->simParticle_ = simParticle;
        }

    private:

        int id_{0};
        float edep_{0};
        float x_{0};
        float y_{0};
        float z_{0};
        float time_{0};

        TRef simParticle_{nullptr};

    ClassDef(SimCalorimeterHit, 1)
};

}

#endif
