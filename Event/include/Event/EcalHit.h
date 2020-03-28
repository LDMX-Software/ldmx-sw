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
            void Clear();

            /** Print a text representation of this object. */
            void Print() const;

            /**
             * Get the cell of the hit from the ID.
             *
             * @return The cell of the hit from the ID.
             */
            int getCell() const;

        private:

            /** The ROOT class definition. */
            ClassDef(EcalHit, 2);
    };
}

#endif /* EVENT_ECALHIT_H_ */
