/**
 * @file SimTrackerHit.h
 * @brief Class which encapsulates information from a hit in a simulated tracking detector
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_SIMTRACKERHIT_H_
#define EVENT_SIMTRACKERHIT_H_

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

// STL
#include <iostream>

namespace ldmx {

/**
 * @class SimTrackerHit
 * @brief Represents a simulated tracker hit in the simulation
 */
class SimTrackerHit: public TObject {

    public:

        /**
         * Class constructor.
         */
        SimTrackerHit();

        /**
         * Class destructor.
         */
        virtual ~SimTrackerHit();

        /**
         * Print a description of this object.
         */
        void Print(Option_t *option = "") const;

        /**
         * Reset the SimTrackerHit object.
         */
        void Clear(Option_t *option = "");

        /**
         * Get the detector ID of the hit.
         * @return The detector ID of the hit.
         */
        int getID() const { return id_; };

        /**
         * Get the geometric layer ID of the hit.
         * @return The layer ID of the hit.
         */
        int getLayerID() const { return layerID_; };

        /**
         * Get the XYZ position of the hit [mm].
         * @return The position of the hit.
         */
        std::vector<float> getPosition() const { return {x_, y_, z_}; };

        /**
         * Get the energy deposited on the hit [MeV].
         * @return The energy deposited on the hit.
         */
        float getEdep() const { return edep_; };

        /**
         * Get the global time of the hit [ns].
         * @return The global time of the hit.
         */
        float getTime() const { return time_; };

        /**
         * Get the path length between the start and end points of the
         * hit [mm].
         * @return The path length of the hit.
         */
        float getPathLength() const { return pathLength_; };

        /**
         * Get the XYZ momentum of the particle at the position at which
         * the hit took place [MeV].
         * @return The momentum of the particle.
         */
        std::vector<double> getMomentum() const { return {px_, py_, pz_}; };

        /**
         * Get the Monte Carlo particle that created the hit.
         * @return The particle that created the hit.
         */
        SimParticle* getSimParticle() const;

        /**
         * Set the detector ID of the hit.
         * @param id The detector ID of the hit.
         */
        void setID(const long id) { this->id_ = id; };

        /**
         * Set the position of the hit [mm].
         * @param x The X position.
         * @param y The Y position.
         * @param z The Z position.
         */
        void setPosition(const float x, const float y, const float z);

        /**
         * Set the energy deposited on the hit [MeV].
         * @param edep The energy deposited on the hit.
         */
        void setEdep(const float edep) { this->edep_ = edep; };

        /**
         * Set the global time of the hit [ns].
         * @param time The global time of the hit.
         */
        void setTime(const float time) { this->time_ = time; };

        /**
         * Set the path length of the hit [mm].
         * @param pathLength The path length of the hit.
         */
        void setPathLength(const float pathLength) { this->pathLength_ = pathLength; };

        /**
         * Set the momentum of the particle at the position at which
         * the hit took place [GeV].
         * @param px The X momentum.
         * @param py The Y momentum.
         * @param pz The Z momentum.
         */
        void setMomentum(const float px, const float py, const float pz);

        /**
         * Set the geometric layer ID of the hit.
         * @param layerID The layer ID of the hit.
         */
        void setLayerID(const int layerID) { this->layerID_ = layerID; };

        /**
         * Set the Monte Carlo particle that created the hit.
         * @param simParticle The particle that created the hit.
         */
        void setSimParticle(SimParticle* simParticle) { this->simParticle_.SetObject(simParticle); };

    private:

        /**
         * The detector ID.
         */
        int id_{0};

        /**
         * The layer ID.
         */
        int layerID_{0};

        /**
         * The energy deposited on the hit.
         */
        float edep_{0};

        /**
         * The global time of the hit.
         */
        float time_{0};

        /**
         * The X momentum.
         */
        float px_{0};

        /**
         * The Y momentum.
         */
        float py_{0};

        /**
         * The Z momentum.
         */
        float pz_{0};

        /**
         * The X position.
         */
        float x_{0};

        /**
         * The Y position.
         */
        float y_{0};

        /**
         * The Z position.
         */
        float z_{0};

        /**
         * The path length of the hit.
         */
        float pathLength_{0};

        /**
         * The particle that caused the hit.
         */
        TRef simParticle_{nullptr};

    /**
     * The ROOT class definition.
     */
    ClassDef(SimTrackerHit, 1);

}; // SimTrackerHit
}

#endif
