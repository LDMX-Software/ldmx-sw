#ifndef EVENT_CALORIMETERHIT_H_
#define EVENT_CALORIMETERHIT_H_

// ROOT
#include "TObject.h"

namespace event {

    /** 
     * @class CalorimeterHit
     * @brief Stores reconstructed hit information from a calorimeter
     *
     * @note This class representes the reconstructed hit information
     * from a calorimeter
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
            amplitude_=amplitude;
        }

        /**
         * Get the calorimetric energy of the hit, corrected for 
         * sampling factors [MeV]
	 * @return The energy of the hit
         */
        float getEnergy() const {
            return energy_;
        }

        /**
         * Set the calorimetric energy of the hit, corrected for 
         * sampling factors [MeV]
	 * @param energy The energy of the hit
         */
        void setEnergy(float energy) {
            energy_ = energy;
        }

        /**
         * Get the time of the hit [ns]
	 * @return The time of the hit
         */
        float getTime() const {
            return time_;
        }

        /**
         * Set the time of the hit [ns]
	 * @param time The time of the hit
         */
        void setTime(float time) {
            time_ = time;
        }

    private:

        int id_{0};
        float amplitude_{0};
        float energy_{0};
        float time_{0};

	/**
	 * The ROOT class definition.
	 */
	ClassDef(CalorimeterHit, 1);
    };

}

#endif /* INCLUDE_EVENT_CALORIMETERHIT_H_ */
