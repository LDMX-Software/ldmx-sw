#ifndef SimApplication_SimParticleBuilder_h
#define SimApplication_SimParticleBuilder_h

// LDMX
#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"
#include "Event/SimParticle.h"

// STL
#include <map>

using event::SimParticle;

namespace sim {

class SimParticleBuilder {

    public:

        typedef std::map<G4int, SimParticle*> SimParticleMap;

        SimParticleBuilder();

        virtual ~SimParticleBuilder();

        void buildSimParticles();

        SimParticle* findSimParticle(G4int);

        void assignTrackerHitSimParticles();

        void assignCalorimeterHitSimParticles();

    private:

        void buildSimParticle(SimParticle* p, Trajectory* info);

    private:

        SimParticleMap particleMap;
        TrackMap* trackMap;
};

}

#endif
