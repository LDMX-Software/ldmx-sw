/**
 *
 * @file SimTrackerHit.h
 * @brief Class used to encapsulate information from a hit in a tracking 
 *        detector.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 *
 */

#ifndef __EVENT_SIMTRACKERHIT_H__
#define __EVENT_SIMTRACKERHIT_H__ 

// C++ StdLib
#include <iostream>
#include <bitset>

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

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

        /** Returns the layer ID where the hit occurred. */
        int getLayerID() const { return layer_id; }; 

        /** Returns the position of the hit in mm. */
        std::vector<float> getPosition() const; 

        /**
         * Returns the position in mm at which a particle begins to deposit 
         * energy in an active volume.
         */
        std::vector<float> getStartPosition() const; 

        /**
         * Returns the position in mm at which a particle stops depositing 
         * energy in an active volume.
         */
        std::vector<float> getEndPosition() const;

        /** Returns the energy deposited on the hit in GeV. */
        float getEdep() const { return edep; };

        /** Returns the time of the hit in ns. */
        float getTime() const { return time; };

        /** 
         * Returns the momentum in GeV of the particle at the position at which
         * the hit took place.
         */
        std::vector<double> getMomentum() const;
        
        /** Returns the Monte Carlo particle that created the hit. */
        SimParticle* getSimParticle() const;

        /** Sets the ID of the hit. */
        void setID(const long id) { this->id = id; };

        /** Sets the layer ID where the hit occurred. */
        void setLayerID(const int layer_id) { this->layer_id = layer_id; };

        /** Set the position of the hit in mm. */
        std::vector<float> setPosition(const float x, const float y, const float z); 
        
        /**
         * Sets the position in mm at which a particle begins to deposit 
         * energy in an active volume.
         */
        void setStartPosition(const float start_x, const float start_y, const float start_z);

        /**
         * Sets the position in mm at which a particle stops depositing 
         * energy in an active volume.
         */
        void setEndPosition(const float end_x, const float end_y, const float end_z);
        
        /** Sets the energy deposited on the hit in GeV. */
        void setEdep(const float edep) { this->edep = edep; };

        /** Sets the time of the hit in ns. */
        void setTime(const float time) { this->time = time; };
        
        /** 
         * Sets the momentum in GeV of the particle at the position at which
         * the hit took place.
         */
        void setMomentum(const float px, const float py, const float pz);

        /** Sets the Monte Carlo particle that created the hit. */
        void setSimParticle(SimParticle* simParticle);

    private:

        /** */
        TRef simParticle{nullptr};

        /** */
        long id{0};
        
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
        float start_x{0};
        
        /** */
        float start_y{0};
        
        /** */
        float start_z{0};

        /** */
        float end_x{0};
        
        /** */
        float end_y{0};
        
        /** */
        float end_z{0};
        
        ClassDef(SimTrackerHit, 1);

}; // SimTrackerHit

#endif // __EVENT_SIMTRACKERHIT_H__ 
