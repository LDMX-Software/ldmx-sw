/**
 * @file EcalHit.h
 * @brief Class that stores reconstructed hit information from the ECAL
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef EVENT_ECALHIT_H_
#define EVENT_ECALHIT_H_

// LDMX
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

        /**
         * Class constructor.
         */
        EcalHit() {;}

        /**
         * Class destructor.
         */
        virtual ~EcalHit() {;}

        /**
         * Print out the object.
         */
        void Print(Option_t *option = "") const;

        /**
         * Get the cell of the hit from the ID.
         * @return The cell of the hit from the ID.
         */
        int getCell() const;

    private:

    /**
     * The ROOT class definition.
     */
    ClassDef(EcalHit, 1);
};

}

#endif /* EVENT_ECALHIT_H_ */
