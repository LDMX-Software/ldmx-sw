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

        /**
         * Information about a contribution to the hit
         * from a step in the associated cell.
         */
        struct Contrib {
            SimParticle* particle{nullptr};
            int pdgCode{0};
            float edep{0};
            float time{0};
        };

        SimCalorimeterHit();

        virtual ~SimCalorimeterHit();

        void Clear(Option_t *option = "");

        void Print(Option_t *option = "") const;

        int getID() {
            return id_;
        }

        void setID(const int id) {
            this->id_ = id;
        }

        float getEdep() {
            return edep_;
        }

        void setEdep(const float edep) {
            this->edep_ = edep;
        }

        std::vector<float> getPosition() const {
            return {x_, y_, z_};
        }

        void setPosition(const float x, const float y, const float z) {
            this->x_ = x;
            this->y_ = y;
            this->z_ = z;
        }

        float getTime() {
            return time_;
        }

        void setTime(const float time) {
            this->time_ = time;
        }

        unsigned getNumberOfContribs() {
            return nContribs_;
        }

        /**
         * Add a hit contribution from a SimParticle.
         */
        void addContrib(SimParticle* simParticle, int pdgCode, float edep, float time);

        /**
         * Get a hit contribution by index.
         */
        Contrib getContrib(int i);

        /**
         * Find the index of a hit contribution from a SimParticle and PDG code.
         */
        int findContribIndex(SimParticle* simParticle, int pdgCode);

        /**
         * Update an existing hit contribution by incrementing its edep and setting the time
         * if the new time is less than the old one.
         */
        void updateContrib(int i, float edep, float time);

    private:

        int id_{0};
        float edep_{0};
        float x_{0};
        float y_{0};
        float z_{0};
        float time_{0};

        TRefArray* simParticleContribs_;
        std::vector<int> pdgCodeContribs_;
        std::vector<float> edepContribs_;
        std::vector<float> timeContribs_;
        unsigned nContribs_{0};

    ClassDef(SimCalorimeterHit, 2)
};

}

#endif
