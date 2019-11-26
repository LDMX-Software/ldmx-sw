/**
 * @file CalorimeterHit.h
 * @brief Class that represents a reconstructed hit in a calorimeter cell within the detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_CALORIMETERHIT_H_
#define EVENT_CALORIMETERHIT_H_

// ROOT
#include "TObject.h"

namespace ldmx {

    /**
     * @class CalorimeterHit
     * @brief Represents a reconstructed hit in a calorimeter cell within the detector
     *
     * @note This class represents the reconstructed hit information
     * from a calorimeter including detector ID, raw amplitude, corrected energy,
     * real space position, and time.
     */
    class CalorimeterHit : public TObject {

        public:

            /**
             * Class constructor.
             */
            CalorimeterHit() { }

            /**
             * Class destructor.
             */
            virtual ~CalorimeterHit() { }

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
            int getID() const { return id_; }

            /**
             * Set the detector ID.
             * @id The detector ID.
             */
            void setID(int id) { id_ = id; }

            /**
             * Get the amplitude of the hit, which is proportional to the
             * signal in the calorimeter cell without sampling factor
             * corrections.  Units depend on the calorimeter.
             * @return The amplitude of the hit
             */
            float getAmplitude() const { return amplitude_; }

            /**
             * Set the amplitude of the hit, which is proportional to the
             * signal in the calorimeter cell without sampling factor
             * corrections.  Units depend on the calorimeter.
             * @param amplitude The amplitude of the hit
             */
            void setAmplitude(float amplitude) { amplitude_ = amplitude; }

            /**
             * Get the calorimetric energy of the hit, corrected for
             * sampling factors [MeV].
             * @return The energy of the hit
             */
            float getEnergy() const { return energy_; }

            /**
             * Set the calorimetric energy of the hit, corrected for
             * sampling factors [MeV].
             * @param energy The energy of the hit
             */
            void setEnergy(float energy) { energy_ = energy; }

            /**
             * Get the time of the hit [ns].
             * @return The time of the hit
             */
            float getTime() const { return time_; }

            /**
             * Set the time of the hit [ns].
             * @param time The time of the hit
             */
            void setTime(float time) { time_ = time; }

            /**
             * Get the x coordinate of the hit.
             * @return the x coordinate of the hit.
             */
            float getX() const { return xpos_; }

            /**
             * Get the y coordinate of the hit.
             * @return the y coordinate of the hit.
             */
            float getY() const { return ypos_; }

            /**
             * Get the z coordinate of the hit.
             * @return the z coordinate of the hit.
             */
            float getZ() const { return zpos_; }

            /**
             * Set the x position this hit.
             * @param x  x-position of centroid hit
             */
            void setXpos(float x) { xpos_ = x; }
            
            /**
             * Set the y position this hit.
             * @param y  y-position of centroid hit
             */
            void setYpos(float y) { ypos_ = y; }

            /**
             * Set the z position this hit.
             * @param z  z-position of centroid hit
             */
            void setZpos(float z) { zpos_ = z; }

            /**
             * Get the layer of the hit from the ID.
             */
            int getLayer() const;

    	    /**
    	     * Get the value of isNoise_.
    	     * @return isNoise_ of hit. 
    	     */
    	    bool isNoise() const { return isNoise_; }

            /** 
             * Set noise flag.
             */
            void setNoise(bool is_noise_=true) { isNoise_ = is_noise_; }

        private:

            /** The detector ID of the hit. */
            int id_{0};

            /** The amplitude value before sampling corrections. */
            float amplitude_{0};

            /** The energy of the hit corrected by a sampling fraction. */
            float energy_{0};

            /** The time of the hit. */
            float time_{0};

            /** The x position of this hit. */
            float xpos_{0};

            /** The x position of this hit. */
            float ypos_{0};
            
            /** The z position of this hit. */
            float zpos_{0};

            /** Flag specifying whether hit is purely from noise. */
            bool isNoise_{false};

            /**
             * The ROOT class definition.
             */
            ClassDef(CalorimeterHit, 1);
    };

}

#endif /* EVENT_CALORIMETERHIT_H_ */
