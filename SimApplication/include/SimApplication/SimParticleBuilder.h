/**
 * @file SimParticleBuilder.h
 * @brief Class for building output SimParticle collection from trajectories
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_SIMPARTICLEBUILDER_H_
#define SIMAPPLICATION_SIMPARTICLEBUILDER_H_

// LDMX
#include "Event/Event.h"
#include "Event/SimParticle.h"
#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"
#include "SimApplication/TrajectoryContainer.h"

// Geant4
#include "G4Event.hh"

// STL
#include <map>

namespace ldmx {

/**
 * @class SimParticleBuilder
 * @brief Builds output SimParticle collection from Trajectory container
 */
class SimParticleBuilder {

    public:

        /**
         * Map of track ID to SimParticles.
         */
        typedef std::map<G4int, SimParticle*> SimParticleMap;

        /**
         * Class constructor.
         */
        SimParticleBuilder();

        /**
         * Class destructor.
         */
        virtual ~SimParticleBuilder();

        /**
         * Set the current Geant4 event.
         * @param anEvent The Geant4 event.
         */
        void setCurrentEvent(const G4Event* anEvent) {
            this->currentEvent_ = const_cast<G4Event*>(anEvent);
        }

        /**
         * Build SimParticle collection into an output event.
         * @param outputEvent The output event.
         */
        void buildSimParticles(Event* outputEvent);

        /**
         * Find a SimParticle by track ID.
         * @param trackID The trackID of the particle.
         */
        SimParticle* findSimParticle(G4int trackID);

    private:

        /**
         * Build a SimParticle from trajectory information.
         * @param info The trajectory information.
         */
        void buildSimParticle(Trajectory* info);
        
        /**
         * Build the SimParticle map from the trajectory container.
         * This will create SimParticles without their information filled.
         * @param trajectories The input trajectory container.
         * @param simParticleColl The output SimParticle collection.
         */
        void buildParticleMap(TrajectoryContainer* trajectories, TClonesArray* simParticleColl);

    private:

        /** The map of track IDs to SimParticles. */
        SimParticleMap particleMap_;

        /** The map of tracks to their parent IDs and Trajectory objects. */
        TrackMap* trackMap_;

        /** The current Geant4 event. */
        G4Event* currentEvent_;

        /** The output SimParticle collection. */
        TClonesArray* outputParticleColl_{new TClonesArray("event::SimParticle", 50)};
};

}

#endif
