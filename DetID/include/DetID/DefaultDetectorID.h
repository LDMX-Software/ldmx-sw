/**
 *
 * @file DefaultDetectorID.h
 * @brief Class that defines a default detector ID
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETID_DEFAULTDETECTORID_H_
#define DETID_DEFAULTDETECTORID_H_

// LDMX
#include "DetID/DetectorID.h"

namespace ldmx {

    /**
     * @class DefaultDetectorID
     * @brief Defines a default detector ID with encoded layer and subdetector values
     *
     * @note
     * This class provides access to a subdetector ID and layer number.
     */
    class DefaultDetectorID : public DetectorID {

        public:

            /**
             * Class constructor which adds layer and subdetector fields to the ID definition.
             */
            DefaultDetectorID();

            /**
             * Get the subdetector value.
             * @return The subdetector value.
             */
            int getSubdetID() {
                return this->getFieldValue(0);
            }

            /**
             * Get the layer value.
             * @return The layer value.
             */
            int getLayerID() {
                return this->getFieldValue(1);
            }
    };

}

#endif
