/**
 * @file EcalDetectorID.h
 * @brief Class that defines an ECal detector ID with a cell number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_HCALDETECTORID_H_
#define DETDESCR_HCALDETECTORID_H_

// LDMX
#include "DetDescr/DefaultDetectorID.h"

namespace ldmx {

    /**
     * @class EcalDetectorID
     * @brief Extension of DefaultDetectorID providing access to ECal cell number in a hex grid
     */
    class HcalID : public DefaultDetectorID {

        public:

            /**
             * Adds a cell field and re-initializes the ID.
             */
            HcalID() {
                this->getFieldList()->push_back(new IDField("section", 2, 12, 14));
                this->getFieldList()->push_back(new IDField("strip", 3, 15, 20));
                init();
            }

            /**
             * Get the value of the 'section' field from the ID.
             * @return The value of the 'strip' field.
             */
            int getSection() {
                return this->getFieldValue(3);
            }

            /**
             * Get the value of the 'strip' field from the ID.
             * @return The value of 'strip' field.
             */
            int getStrip() {
                return this->getFieldValue(4);
            }
    };
}

#endif
