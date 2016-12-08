#ifndef DETDESCR_ECALDETECTORID_H_
#define DETDESCR_ECALDETECTORID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace detdescr {

/**
 * @class DefaultDetectorID
 * @brief Defines a default detector ID with encoded layer and subdetector values.
 */
class EcalDetectorID : public DetectorID {

    public:

        /**
         * Class constructor which adds layer and subdetector fields to the ID definition.
         */
        EcalDetectorID();

        /**
         * Get the subdetector value.
         * @return the subdetector value
         */
        int getSubdetID() {
            return this->getFieldValue(0);
        }

        /**
         * Get the layer value.
         * @return the layer value
         */
        int getLayerID() {
            return this->getFieldValue(1);
        }
};

}

#endif
