#ifndef EVENT_SIMPARTICLE_H_
#define EVENT_SIMPARTICLE_H_ 1

// STL
#include <vector>

// ROOT
#include "TObject.h"
#include "TRefArray.h"

class SimParticle: public TObject {

    public:

        SimParticle();

        virtual ~SimParticle();

        void Print(Option_t *option) const;

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

        /**
         * Returns a vector with pointers copied from the TRefArray.
         */
        //std::vector<SimParticle*> daughters();
        /**
         * Returns a vector with pointers copied from the TRefArray.
         */
        //std::vector<SimParticle*> parents();

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

        //TRefArray* _daughters;
        //TRefArray* _parents;

        ClassDef(SimParticle, 1);
};

#endif
