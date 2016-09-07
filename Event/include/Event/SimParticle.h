#ifndef EVENT_SIMPARTICLE_H_
#define EVENT_SIMPARTICLE_H_ 1

// STL
#include <vector>

// ROOT
#include "TObject.h"

class SimParticle : public TObject {

public:

    SimParticle();

    virtual ~SimParticle();

    double energy();

    int pdg();

    int simStatus();

    int genStatus();

    float time();

    double* vertex();

    double* endPoint();

    double* momentum();

    double mass();

    std::vector<SimParticle*>& daughters();

    double setEnergy(double energy);

    double setPdg(int pdg);

    void setSimStatus(int simStatus);

    void setGenStatus(int genStatus);

    void setTime(float time);

    void setVertex(double x, double y, double z);

    void setEndPoint(double x, double y, double z);

    void setMomentum(double px, double py, double pz);

    void setMass(double mass);

    void addDaughter(SimParticle* daughter);

    ClassDef(SimParticle, 1)

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
    std::vector<SimParticle*> _daughters;
};

#endif
