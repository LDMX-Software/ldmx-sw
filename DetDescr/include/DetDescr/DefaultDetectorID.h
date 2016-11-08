#ifndef DETDESCR_DEFAULTDETECTORID_H_
#define DETDESCR_DEFAULTDETECTORID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace detdescr {

/**
 * @class DefaultDetectorID
 * @brief Defines a default detector ID with encoded layer and subdetector values.
 */
class DefaultDetectorID : public DetectorID {

    public:

        /**
         * Class constructor which adds layer and subdetector fields to the ID definition.
         */
        DefaultDetectorID();

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
