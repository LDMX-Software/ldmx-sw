/**
 * @file HcalHit.h
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef EVENT_HCALHIT_H_
#define EVENT_HCALHIT_H_

// LDMX
#include "Event/CalorimeterHit.h"

#include <ostream>

namespace ldmx {

    /**
     * @class HcalHit
     * @brief Stores reconstructed hit information from the HCAL
     *
     * @note This class represents the reconstructed hit information
     * from the HCAL, providing particular information for the HCAL,
     * above and beyond what is available in the CalorimeterHit.
     */
    class HcalHit : public CalorimeterHit {

        public:

            /**
             * Class constructor.
             */
            HcalHit() {
            }

            /**
             * Class destructor.
             */
            virtual ~HcalHit() {
            }

            /**
             * Clear the data in the object.
             */
            void Clear();

            /**
             * Print out the object.
             */
            void Print(std::ostream& o) const;

            /**
             * Get the number of photoelectrons estimated for this hit.
             * @return Number of photoelectrons, including noise which affects the estimate.
             */
            float getPE() const {
                return pe_;
            }

            /**
             * Get the minimum number of photoelectrons estimated for this hit if two sided readout.
             * @return Minimum number of photoelectrons, including noise which affects the estimate.
             */
            float getMinPE() const {
                return minpe_;
            }            

            /**
             * Get the x coordinate of the hit.
             * @return the x coordinate of the hit.
             */
            float getX() const {
                return xpos_;
            }

            /**
             * Get the y coordinate of the hit.
             * @return the y coordinate of the hit.
             */
            float getY() const {
                return ypos_;
            }
            /**
             * Get the z coordinate of the hit.
             * @return the z coordinate of the hit.
             */
            float getZ() const {
                return zpos_;
            }

            /// Decode the section associated with the hit from the ID. 
            virtual int getSection() const { return (getID() & 0x7000) >> 12; }

            /// Decode the strip associated with the hit from the ID. 
            virtual int getStrip() const { return (getID() & 0x7F8000) >> 15; }

    	    /**
    	     * Get the value of isNoise_.
    	     * @return isNoise_ of hit. 
    	     */
    	    bool getNoise() const {
    	        return isNoise_;
    	    }

            /**
             * Set the number of photoelectrons estimated for this hit.
             * @param pe Number of photoelectrons, including noise which affects the estimate.
             */
            void setPE(float pe) {
                pe_ = pe;
            }

            /**
             * Set the minimum number of photoelectrons estimated for this hit.
             * @param pe Minimum number of photoelectrons, including noise which affects the estimate.
             */
            void setMinPE(float minpe) {
                minpe_ = minpe;
            }            

            /**
             * Set the x position this hit.
             * @param x  x-position of centroid hit
             */
            void setXpos(float x) {
                xpos_ = x;
            }
            
            /**
             * Set the y position this hit.
             * @param y  y-position of centroid hit
             */
            void setYpos(float y) {
                ypos_ = y;
            }

            /**
             * Set the z position this hit.
             * @param z  z-position of centroid hit
             */
            void setZpos(float z) {
                zpos_ = z;
            }

            /** 
             * Set noise flag.
             */
            void setNoise(bool is_noise_=true){
                isNoise_ = is_noise_;
            }

        private:

            /** The number of PE estimated for this hit. */
            float pe_{0};

            /** The minimum number of PE estimated for this hit, different from pe_ when you have two ended readout */
            float minpe_{-99};

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
            ClassDef(HcalHit, 1);
    };

}

#endif /* EVENT_HCALHIT_H_ */
