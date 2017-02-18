/**
 * @file HcalHit.h
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef EVENT_HCALHIT_H_
#define EVENT_HCALHIT_H_

// LDMX
#include "Event/CalorimeterHit.h"

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
        HcalHit() {;}

        /**
         * Class destructor.
         */
        virtual ~HcalHit() {;}

        /**
         * Clear the data in the object.
         */
        void Clear(Option_t *option = "");

        /**
         * Print out the object.
         */
        void Print(Option_t *option = "") const;

        /**
         * Get the number of photoelectrons estimated for this hit.
         * @return Number of photoelectrons, including noise which affects the estimate.
         */
        float getPE() const {
            return pe_;
        }

        /**
         * Set the number of photoelectrons estimated for this hit.
         * @param pe Number of photoelectrons, including noise which affects the estimate.
         */
        void setPE(float pe) {
            pe_ = pe;
        }

    private:

        /** The number of PE estimated for this hit. */
        float pe_{0};

        /**
         * The ROOT class definition.
         */
        ClassDef(HcalHit, 1);
};

}

#endif /* EVENT_HCALHIT_H_ */
