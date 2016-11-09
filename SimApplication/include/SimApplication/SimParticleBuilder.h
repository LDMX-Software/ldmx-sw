#ifndef SIMAPPLICATION_SIMPARTICLEBUILDER_H_
#define SIMAPPLICATION_SIMPARTICLEBUILDER_H_

// LDMX
#include "Event/Event.h"
#include "Event/SimParticle.h"
#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"

// Geant4
#include "G4Event.hh"

// STL
#include <map>

using event::SimParticle;
using event::Event;

namespace sim {

class SimParticleBuilder {

    public:

        typedef std::map<G4int, SimParticle*> SimParticleMap;

        SimParticleBuilder();

        virtual ~SimParticleBuilder();

        void setCurrentEvent(const G4Event* anEvent) {
            this->currentEvent_ = const_cast<G4Event*>(anEvent);
        }

        void buildSimParticles(Event* outputEvent);

        SimParticle* findSimParticle(G4int trackID);

        void assignTrackerHitSimParticles();

        void assignCalorimeterHitSimParticles();

    private:

        void buildSimParticle(SimParticle* p, Trajectory* info);

    private:

        SimParticleMap particleMap_;
        TrackMap* trackMap_;
        G4Event* currentEvent_;
};

}

#endif
