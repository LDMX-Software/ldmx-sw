/**
 *
 * @file SimTrackerHit.h
 * @brief Class used to encapsulate information from a hit in a
 *        simulated tracking detector.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_SIMTRACKERHIT_H_
#define EVENT_SIMTRACKERHIT_H_

// STL
#include <iostream>
#include <bitset>

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

namespace event {

class SimTrackerHit: public TObject {

    public:

        /** Constructor */
        SimTrackerHit();

        /** Destructor */
        virtual ~SimTrackerHit();

        /** Print a description of this object. */
        void Print(Option_t *option = "") const;

        /** Reset the SimTrackerHit object. */
        void Clear(Option_t *option = "");

        /** Return the ID of the hit. */
        int getID() const { return id_; };

        /**
         * Get the geometric layer ID of the hit.
         */
        int getLayerID() const { return layerID_; };

        /** Return the position of the hit in mm. */
        std::vector<float> getPosition() const { return {x_, y_, z_}; };

        /** Return the energy deposited on the hit in GeV. */
        float getEdep() const { return edep_; };

        /** Return the time of the hit in ns. */
        float getTime() const { return time_; };

        /**
         * Return the path length between the start and end points of the
         * hit in mm.
         */
        float getPathLength() const { return pathLength_; };

        /**
         * Return the momentum in GeV of the particle at the position at which
         * the hit took place.
         */
        std::vector<double> getMomentum() const { return {px_, py_, pz_}; };

        /** Return the Monte Carlo particle that created the hit. */
        SimParticle* getSimParticle() const;

        /** Set the ID of the hit. */
        void setID(const long id) { this->id_ = id; };

        /** Set the position of the hit in mm. */
        void setPosition(const float x, const float y, const float z);

        /** Set the energy deposited on the hit in GeV. */
        void setEdep(const float edep) { this->edep_ = edep; };

        /** Set the time of the hit in ns. */
        void setTime(const float time) { this->time_ = time; };

        /**
         * Set the path length of the hit in mm.
         */
        void setPathLength(const float path_length) { this->pathLength_ = path_length; };

        /**
         * Set the momentum in GeV of the particle at the position at which
         * the hit took place.
         */
        void setMomentum(const float px, const float py, const float pz);

        /**
         * Set the geometric layer ID of the hit.
         */
        void setLayerID(const int layerID) { this->layerID_ = layerID; };

        /** Set the Monte Carlo particle that created the hit. */
        void setSimParticle(SimParticle* simParticle) { this->simParticle_.SetObject(simParticle); };

    private:

        int id_{0};
        int layerID_{0};
        float edep_{0};
        float time_{0};
        float px_{0};
        float py_{0};
        float pz_{0};
        float x_{0};
        float y_{0};
        float z_{0};
        float pathLength_{0};

        TRef simParticle_{nullptr};

        ClassDef(SimTrackerHit, 1);

}; // SimTrackerHit
}

#endif // Event_SimTrackerHit_h
