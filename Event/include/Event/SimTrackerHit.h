/**
 *
 * @file SimTrackerHit.h
 * @brief Class used to encapsulate information from a hit in a tracking 
 *        detector.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_SIM_TRACKER_HIT_h
#define EVENT_SIM_TRACKER_HIT_h

// C++ StdLib
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

            /** Returns the ID of the hit. */
            int getID() const { return id; };

            /**
             * Get the geometric layer ID of the hit.
             */
            int getLayerID() const { return layer_id; };

            /** Returns the position of the hit in mm. */
            std::vector<float> getPosition() const { return {x, y, z}; }; 

            /** Returns the energy deposited on the hit in GeV. */
            float getEdep() const { return edep; };

            /** Returns the time of the hit in ns. */
            float getTime() const { return time; };

            float getPathLength() const { return path_length; };

            /** 
             * Returns the momentum in GeV of the particle at the position at which
             * the hit took place.
             */
            std::vector<double> getMomentum() const { return {px, py, pz}; };

            /** Returns the Monte Carlo particle that created the hit. */
            SimParticle* getSimParticle() const;

            /** Sets the ID of the hit. */
            void setID(const long id) { this->id = id; };

            /** Set the position of the hit in mm. */
            void setPosition(const float x, const float y, const float z); 

            /** Sets the energy deposited on the hit in GeV. */
            void setEdep(const float edep) { this->edep = edep; };

            /** Sets the time of the hit in ns. */
            void setTime(const float time) { this->time = time; };

            void setPathLength(const float path_length) { this->path_length = path_length; };

            /** 
             * Sets the momentum in GeV of the particle at the position at which
             * the hit took place.
             */
            void setMomentum(const float px, const float py, const float pz);

            /**
             * Set the geometric layer ID of the hit.
             */
            void setLayerID(const int layer_id) { this->layer_id = layer_id; };

            /** Sets the Monte Carlo particle that created the hit. */
            void setSimParticle(SimParticle* simParticle) { this->simParticle = simParticle; };

        private:

            /** */
            TRef simParticle{nullptr};

            /** */
            int id{0};

            /** */
            int layer_id{0};

            /** */
            float edep{0};

            /** */
            float time{0};

            /** */
            float px{0};

            /** */
            float py{0};

            /** */
            float pz{0};

            /** */
            float x{0};

            /** */
            float y{0};

            /** */
            float z{0};

            /** */
            float path_length{0};

            ClassDef(SimTrackerHit, 1);

    }; // SimTrackerHit
}

#endif // EVENT_SIM_TRACKER_HIT_H
