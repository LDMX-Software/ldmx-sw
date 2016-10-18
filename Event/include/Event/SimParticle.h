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

        double getEnergy();

        int getPdg();

        int getSimStatus();

        int getGenStatus();

        float getTime();

        double* getVertex();

        double* getEndPoint();

        double* getMomentum();

        double getMass();

        double getCharge();

        std::vector<SimParticle*> getDaughters();

        std::vector<SimParticle*> getParents();

        void setEnergy(double energy);

        void setPdg(int pdg);

        void setSimStatus(int simStatus);

        void setGenStatus(int genStatus);

        void setTime(float time);

        void setVertex(double x, double y, double z);

        void setEndPoint(double x, double y, double z);

        void setMomentum(double px, double py, double pz);

        void setMass(double mass);

        void setCharge(double charge);

        void addDaughter(SimParticle* daughter);

        void addParent(SimParticle* parent);

    private:

        double energy;
        int pdg;
        int simStatus;
        int genStatus;
        float time;
        double vertex[3];
        double endPoint[3];
        double momentum[3];
        double mass;
        double charge;

        TRefArray* daughters;
        TRefArray* parents;

        ClassDef(SimParticle, 1);
};

}

#endif
