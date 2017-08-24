/**
 * @file HcalID.h
 * @brief Class that defines an HCal sensitive detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_HCALID_H_
#define DETDESCR_HCALID_H_

// LDMX
#include "DetDescr/DefaultDetectorID.h"

namespace ldmx {

    /**
     * Encodes the section of the HCal based on the 'section' field value.
     */
    enum HcalSection {
        BACK = 0,
        TOP = 1,
        BOTTOM = 2,
        LEFT = 3,
        RIGHT = 4
    };

    /**
     * @class HcalID
     * @brief Implements sensitive detector for HCal subdetector
     */
    class HcalID : public DefaultDetectorID {

        public:

            HcalID() {
                this->getFieldList()->push_back(new IDField("section", 2, 12, 14));
                this->getFieldList()->push_back(new IDField("strip", 3, 15, 22));
                init();
            }

            /**
             * Get the value of the 'section' field from the ID.
             * @return The value of the 'strip' field.
             */
            int getSection() {
                return this->getFieldValue(2);
            }

            /**
             * Get the value of the 'strip' field from the ID.
             * @return The value of 'strip' field.
             */
            int getStrip() {
                return this->getFieldValue(3);
            }
    };
}

#endif
