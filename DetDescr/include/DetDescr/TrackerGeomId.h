#ifndef DETDESCR_TRACKERGEOMID_H_
#define DETDESCR_TRACKERGEOMID_H_ 1

// LDMX
#include "DetDescr/DetectorId.h"

/**
 * Detector ID for tracker geometry.
 */
class TrackerGeomId : public DetectorId {

    public:

        TrackerGeomId() {
            idFieldList = new IdField::IdFieldList;
            idFieldList->push_back(new IdField("subdet", 0, 0, 3));
            idFieldList->push_back(new IdField("layer", 1, 4, 11));
            // TODO: add sensor number, etc.
        }

        virtual ~TrackerGeomId() {
        }
};

#endif
