#ifndef EVENT_SIMPARTICLE_H_
#define EVENT_SIMPARTICLE_H_

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

        void Clear(Option_t *option = "");

        void Print(Option_t *option = "") const;

        float getEnergy() {
            return energy_;
        }

        int getPdg() {
            return pdg_;
        }

        int getGenStatus() {
            return genStatus_;
        }

        float getTime() {
            return time_;
        }

        std::vector<float> getVertex() {
            return {x_, y_, z_};
        }

        std::vector<float> getEndPoint() {
            return {endX_, endY_, endZ_};
        }

        std::vector<float> getMomentum() {
            return {px_, py_, pz_};
        }

        float getMass() {
            return mass_;
        }

        float getCharge() {
            return charge_;
        }

        std::vector<SimParticle*> getDaughters();

        std::vector<SimParticle*> getParents();

        void setEnergy(const float energy) {
            this->energy_ = energy;
        }

        void setPdg(const int pdg) {
            this->pdg_ = pdg;
        }

        void setGenStatus(const int genStatus) {
            this->genStatus_ = genStatus;
        }

        void setTime(const float time) {
            this->time_ = time;
        }

        void setVertex(const float x, const float y, const float z) {
            this->x_ = x;
            this->y_ = y;
            this->z_ = z;
        }

        void setEndPoint(const float endX, const float endY, const float endZ) {
            this->endX_ = endX;
            this->endY_ = endY;
            this->endZ_ = endZ;
        }

        void setMomentum(const float px, const float py, const float pz) {
            this->px_ = px;
            this->py_ = py;
            this->pz_ = pz;
        }

        void setMass(const float mass) {
            this->mass_ = mass;
        }

        void setCharge(const float charge) {
            this->charge_ = charge;
        }

        void addDaughter(SimParticle* daughter) {
            daughters_->Add(daughter);
        }

        void addParent(SimParticle* parent) {
            parents_->Add(parent);
        }

    private:

        double energy_{0};
        int pdg_{0};
        int genStatus_{-1};
        float time_{0};
        float x_{0};
        float y_{0};
        float z_{0};
        float endX_{0};
        float endY_{0};
        float endZ_{0};
        float px_{0};
        float py_{0};
        float pz_{0};
        float mass_{0};
        float charge_{0};

        TRefArray* daughters_;
        TRefArray* parents_;

        ClassDef(SimParticle, 1);
};

}

#endif
