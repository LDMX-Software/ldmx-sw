#ifndef Event_SimCalorimeterHit_h
#define Event_SimCalorimeterHit_h

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

class SimCalorimeterHit: public TObject {

    public:

        SimCalorimeterHit();

        virtual ~SimCalorimeterHit();

        void Print(Option_t *option = "") const;

        int getID();

        double getEdep();

        double* getPosition();

        float getTime();

        SimParticle* getSimParticle();

        void setID(long id);

        void setEdep(double edep);

        void setPosition(double x, double y, double z);

        void setTime(float time);

        void setSimParticle(SimParticle*);

    private:

        int id;
        double edep;
        double position[3];
        float time;

        TRef simParticle;

    ClassDef(SimCalorimeterHit, 1)
};

#endif
