#include "DetDescr/DefaultDetectorID.h"

namespace detdescr {

DefaultDetectorID::DefaultDetectorID() : DetectorID() {
    IDField::IDFieldList* fieldList = new IDField::IDFieldList();
    fieldList->push_back(new IDField("subdet", 0, 0, 3));
    fieldList->push_back(new IDField("layer", 1, 4, 11));

    setFieldList(fieldList);
}

}
