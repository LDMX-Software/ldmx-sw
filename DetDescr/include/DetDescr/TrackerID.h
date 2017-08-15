/**
 * @file EcalDetectorID.h
 * @brief Class that defines an ECal detector ID with a cell number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_TRACKERID_H_
#define DETDESCR_TRACKERID_H_

// LDMX
#include "DetDescr/DefaultDetectorID.h"

namespace ldmx {

    /**
     * @class EcalDetectorID
     * @brief Extension of DefaultDetectorID providing access to ECal cell number in a hex grid
     */
    class TrackerID : public DefaultDetectorID {

        public:

            /**
             * Adds a cell field and re-initializes the ID.
             */
            TrackerID() {
                this->getFieldList()->push_back(new IDField("module", 2, 12, 16));
                init();
            }

            /**
             * Get the value of the cell field from the ID.
             * @return The value of the cell field.
             */
            int getModule() {
                return this->getFieldValue(2);
            }
    };
}

#endif
