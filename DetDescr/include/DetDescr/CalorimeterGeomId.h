#ifndef DETDESCR_CALORIMETERGEOMID_H_
#define DETDESCR_CALORIMETERGEOMID_H_ 1

// LDMX
#include "DetDescr/DetectorId.h"

/**
 * Detector ID for calorimeter geometry.
 */
class CalorimeterGeomId : public DetectorId {

    public:

        CalorimeterGeomId() {
            idFieldList = new IdField::IdFieldList;
            idFieldList->push_back(new IdField("subdet", 0, 0, 3));
            idFieldList->push_back(new IdField("layer", 1, 4, 11));
            // TODO: add X and V, sensor, etc.
        }

        virtual ~CalorimeterGeomId() {
        }
};

#endif
