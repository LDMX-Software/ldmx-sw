#ifndef EVENT_SIMPARTICLE_H_
#define EVENT_SIMPARTICLE_H_ 1

// STL
#include <vector>

// ROOT
#include "TObject.h"
#include "TRefArray.h"

class SimParticle : public TObject {

public:

    SimParticle();

    virtual ~SimParticle();

    void Print(Option_t *option) const;

    double energy();

    int pdg();

    int simStatus();

    int genStatus();

    float time();

    double* vertex();

    double* endPoint();

    double* momentum();

    double mass();

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

    void addDaughter(SimParticle* daughter);

    void addParent(SimParticle* parent);

private:

    double _energy;
    int _pdg;
    int _simStatus;
    int _genStatus;
    float _time;
    double _vertex[3];
    double _endPoint[3];
    double _momentum[3];
    double _mass;

    //TRefArray* _daughters;
    //TRefArray* _parents;

    ClassDef(SimParticle, 1)
};

#endif
