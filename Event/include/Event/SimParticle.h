#ifndef Event_SimParticle_h
#define Event_SimParticle_h

// STL
#include <vector>

// ROOT
#include "TObject.h"
#include "TRefArray.h"

namespace event {

class SimParticle: public TObject {

    public:

        SimParticle();

        virtual ~SimParticle();

        void Print(Option_t *option = "") const;

        float getEnergy() {
            return energy;
        }

        int getPdg() {
            return pdg;
        }

        int getGenStatus() {
            return genStatus;
        }

        float getTime() {
            return time;
        }

        std::vector<float> getVertex() {
            return {x, y, z};
        }

        std::vector<float> getEndPoint() {
            return {endX, endY, endZ};
        }

        std::vector<float> getMomentum() {
            return {px, py, pz};
        }

        float getMass() {
            return mass;
        }

        float getCharge() {
            return charge;
        }

        std::vector<SimParticle*> getDaughters();

        std::vector<SimParticle*> getParents();

        void setEnergy(const float energy) {
            this->energy = energy;
        }

        void setPdg(const int pdg) {
            this->pdg = pdg;
        }

        void setGenStatus(const int genStatus) {
            this->genStatus = genStatus;
        }

        void setTime(const float time) {
            this->time = time;
        }

        void setVertex(const float x, const float y, const float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        void setEndPoint(const float endX, const float endY, const float endZ) {
            this->endX = endX;
            this->endY = endY;
            this->endZ = endZ;
        }

        void setMomentum(const float px, const float py, const float pz) {
            this->px = px;
            this->py = py;
            this->pz = pz;
        }

        void setMass(const float mass) {
            this->mass = mass;
        }

        void setCharge(const float charge) {
            this->charge = charge;
        }

        void addDaughter(SimParticle* daughter) {
            daughters->Add(daughter);
        }

        void addParent(SimParticle* parent) {
            parents->Add(parent);
        }

    private:

        double energy{0};
        int pdg{0};
        int genStatus{-1};
        float time{0};
        float x{0};
        float y{0};
        float z{0};
        float endX{0};
        float endY{0};
        float endZ{0};
        float px{0};
        float py{0};
        float pz{0};
        float mass{0};
        float charge{0};

        TRefArray* daughters;
        TRefArray* parents;

        ClassDef(SimParticle, 1);
};

}

#endif
