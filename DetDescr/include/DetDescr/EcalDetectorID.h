#ifndef DETDESCR_ECALDETECTORID_H_
#define DETDESCR_ECALDETECTORID_H_

// LDMX
#include "DetDescr/DefaultDetectorID.h"

namespace detdescr {

class EcalDetectorID : public DefaultDetectorID {

    public:

        /**
         * Adds a cell field and re-initializes the ID.
         */
        EcalDetectorID() {
            this->getFieldList()->push_back(new IDField("cell", 2, 12, 31));
            init();
        }

        /**
         * Get the value of the cell field from the ID.
         * @return The value of the cell field.
         */
        int getCellID() {
            return this->getFieldValue(2);
        }
};

}

#endif
