/**
 * @file SimParticleBuilder.h
 * @brief Class for building output SimParticle collection from trajectories
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_SIMPARTICLEBUILDER_H_
#define SIMCORE_SIMPARTICLEBUILDER_H_

// LDMX
#include "Event/EventConstants.h"
#include "Framework/Event.h"
#include "Event/SimParticle.h"
#include "SimCore/TrackMap.h"
#include "SimCore/Trajectory.h"

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

        private:

            /** The map of tracks to their parent IDs and Trajectory objects. */
            TrackMap* trackMap_;

            /** The current Geant4 event. */
            G4Event* currentEvent_;

    };

}

#endif
