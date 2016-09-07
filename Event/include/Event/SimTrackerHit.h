#ifndef EVENT_SIMTRACKERHIT_H_
#define EVENT_SIMTRACKERHIT_H_ 1

// ROOT
#include "TObject.h"

// LDMX
#include "Event/SimParticle.h"

/**
 * Hit information from a simulated tracking detector.
 */
class SimTrackerHit: public TObject {

public:

    SimTrackerHit();

    virtual ~SimTrackerHit();

    long id();

    double* startPosition();

    double* endPosition();

    float edep();

    float time();

    float* momentum();

    void setId(long id);

    void setStartPosition(double x, double y, double z);

    void setEndPosition(double x, double y, double z);

    void setEdep(float edep);

    void setTime(float time);

    void setMomentum(float px, float py, float pz);

    ClassDef(SimTrackerHit, 1)

private:

    long _id;
    double _startPosition[3];
    double _endPosition[3];
    float _edep;
    float _time;
    float _momentum[3];
};

#endif
