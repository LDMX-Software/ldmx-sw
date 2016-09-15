#ifndef EVENT_SIMTRACKERHIT_H_
#define EVENT_SIMTRACKERHIT_H_ 1

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

/**
 * Hit information from a simulated tracking detector.
 */
class SimTrackerHit: public TObject {

    public:

        SimTrackerHit();

        virtual ~SimTrackerHit();

        void Print(Option_t *option = "") const;

        int getId();

        double* getStartPosition();

        double* getEndPosition();

        float getEdep();

        float getTime();

        float* getMomentum();

        void setId(long id);

        void setStartPosition(double x, double y, double z);

        void setEndPosition(double x, double y, double z);

        void setEdep(float edep);

        void setTime(float time);

        void setMomentum(float px, float py, float pz);

        void setSimParticle(SimParticle* simParticle);

    private:

        long id;
        double startPosition[3];
        double endPosition[3];
        float edep;
        float time;
        float momentum[3];
        //TRef _simParticleRef;

        ClassDef(SimTrackerHit, 1);
};

#endif
