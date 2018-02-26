/**
 * @file EcalHit.h
 * @brief Class that stores reconstructed hit information from the ECAL
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef EVENT_ECALHIT_H_
#define EVENT_ECALHIT_H_

//----------//
//   LDMX   //
//----------//
#include "Event/CalorimeterHit.h"

namespace ldmx {

    /**
     * @class EcalHit
     * @brief Stores reconstructed hit information from the ECAL
     *
     * @note This class represents the reconstructed hit information
     * from the ECAL, providing particular information for the ECAL,
     * above and beyond what is available in the CalorimeterHit.
     */
    class EcalHit : public CalorimeterHit {

        public:

            /** Constructor. */
            EcalHit() {}

            /** Destructor. */
            virtual ~EcalHit() {}
            
            /** Clear the data in the object. */
            void Clear(Option_t *option = "");

            /** Print a text representation of this object. */
            void Print(Option_t *option = "") const;

            /**
             * Get the cell of the hit from the ID.
             *
             * @return The cell of the hit from the ID.
             */
            int getCell() const;

            /** Denote this hit as a noise hit. */
            void setNoiseHit(bool isNoise = true) { isNoise_ = isNoise; }

            /** Check whether this hit is due to noise. */
            bool isNoise() { return isNoise_; }

        private:

            /** Flag denoting whether a hit was due to noise. */
            bool isNoise_{false}; 

            /** The ROOT class definition. */
            ClassDef(EcalHit, 2);
    };
}

#endif /* EVENT_ECALHIT_H_ */
