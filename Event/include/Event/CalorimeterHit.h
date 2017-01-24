/**
 * @file CalorimeterHit.h
 * @brief Class that represents a reconstructed hit in a calorimeter cell within the detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_CALORIMETERHIT_H_
#define EVENT_CALORIMETERHIT_H_

// ROOT
#include "TObject.h"

namespace event {

/**
 * @class CalorimeterHit
 * @brief Represents a reconstructed hit in a calorimeter cell within the detector
 *
 * @note This class represents the reconstructed hit information
 * from a calorimeter including detector ID, raw amplitude, corrected energy
 * and time.
 */
class CalorimeterHit : public TObject {

    public:

        /**
         * Class constructor.
         */
        CalorimeterHit() {;}

        /**
         * Class destructor.
         */
        virtual ~CalorimeterHit() {;}

        /**
         * Clear the data in the object.
         */
        void Clear(Option_t *option = "");

        /**
         * Print out the object.
         */
        void Print(Option_t *option = "") const;

        /**
         * Get the detector ID.
         * @return The detector ID.
         */
        int getID() const {
            return id_;
        }

        /**
         * Set the detector ID.
         * @id The detector ID.
         */
        void setID(int id) {
            id_ = id;
        }

        /**
         * Get the amplitude of the hit, which is proportional to the
         * signal in the calorimeter cell without sampling factor
         * corrections.  Units depend on the calorimeter.  
         * @return The amplitude of the hit
         */
        float getAmplitude() const {
            return amplitude_;
        }

        /**
         * Set the amplitude of the hit, which is proportional to the
         * signal in the calorimeter cell without sampling factor
         * corrections.  Units depend on the calorimeter.  
         * @param amplitude The amplitude of the hit
         */
        void setAmplitude(float amplitude) {
            amplitude_ = amplitude;
        }

        /**
         * Get the calorimetric energy of the hit, corrected for 
         * sampling factors [MeV].
         * @return The energy of the hit
         */
        float getEnergy() const {
            return energy_;
        }

        /**
         * Set the calorimetric energy of the hit, corrected for 
         * sampling factors [MeV].
         * @param energy The energy of the hit
         */
        void setEnergy(float energy) {
            energy_ = energy;
        }

        /**
         * Get the time of the hit [ns].
         * @return The time of the hit
         */
        float getTime() const {
            return time_;
        }

        /**
         * Set the time of the hit [ns].
         * @param time The time of the hit
         */
        void setTime(float time) {
            time_ = time;
        }

        /**
         * Get the layer of the hit from the ID.
         */
        int getLayer() const;

    private:

        /** The detector ID of the hit. */
        int id_{0};

        /** The amplitude value before sampling corrections. */
        float amplitude_{0};

        /** The energy of the hit corrected by a sampling fraction. */
        float energy_{0};

        /** The time of the hit. */
        float time_{0};

        /**
         * The ROOT class definition.
         */
        ClassDef(CalorimeterHit, 1);
};

}

#endif /* EVENT_CALORIMETERHIT_H_ */
