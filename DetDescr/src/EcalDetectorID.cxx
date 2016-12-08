#include "../include/DetDescr/EcalDetectorID.h"

namespace detdescr {

EcalDetectorID::EcalDetectorID() :
        DetectorID() {
    IDField::IDFieldList* fieldList = new IDField::IDFieldList();
    fieldList->push_back(new IDField("subdet", 0, 0, 3));
    fieldList->push_back(new IDField("layer", 1, 4, 11));
    fieldList->push_back(new IDField("cellid", 2, 12, 31));

    setFieldList(fieldList);
}

}
