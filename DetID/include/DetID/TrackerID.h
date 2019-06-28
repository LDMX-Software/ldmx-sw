/**
 * @file TrackerID.h
 * @brief Class that defines a Tracker detector ID with a module number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETID_TRACKERID_H_
#define DETID_TRACKERID_H_

// LDMX
#include "DetID/DefaultDetectorID.h"

namespace ldmx {

    /**
     * @class TrackerID
     * @brief Extension of DefaultDetectorID providing access to module number for tracker IDs
     */
    class TrackerID : public DefaultDetectorID {

        public:

            /**
             * Add a module field and reinitialize the ID.
             */
            TrackerID() {
                this->getFieldList()->push_back(new IDField("module", 2, 12, 16));
                init();
            }

            /**
             * Get the value of the module field from the ID.
             * @return The value of the module field.
             */
            int getModule() {
                return this->getFieldValue(2);
            }
    };
}

#endif
