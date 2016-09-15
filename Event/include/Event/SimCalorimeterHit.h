#ifndef EVENT_SIMCALORIMETERHIT_H_
#define EVENT_SIMCALORIMETERHIT_H_ 1

// ROOT
#include "TObject.h"

class SimCalorimeterHit: public TObject {

    public:

        SimCalorimeterHit();

        virtual ~SimCalorimeterHit();

        void Print(Option_t *option = "") const;

        int getId();

        double getEdep();

        double* getPosition();

        void setId(long id);

        void setEdep(double edep);

        void setPosition(double x, double y, double z);

        // TODO: sim particle contributions

    private:

        int id;
        double edep;
        double position[3];

    ClassDef(SimCalorimeterHit, 1)
};

#endif
