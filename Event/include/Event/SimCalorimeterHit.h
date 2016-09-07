#ifndef EVENT_SIMCALORIMETERHIT_H_
#define EVENT_SIMCALORIMETERHIT_H_ 1

// ROOT
#include "TObject.h"

class SimCalorimeterHit : public TObject {

public:

    SimCalorimeterHit();

    virtual ~SimCalorimeterHit();

    void Print(Option_t *option="") const;

    long id();

    double edep();

    double* position();

    void setId(long id);

    void setEdep(double edep);

    void setPosition(double x, double y, double z);

    // TODO: sim particle contribution

private:

    long _id;
    double _edep;
    double _position[3];

    ClassDef(SimCalorimeterHit, 1)
};

#endif
