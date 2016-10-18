// LDMX
#include "DetDescr/DetectorID.h"

// STL
#include <bitset>
#include <iostream>
#include <stdexcept>

using detdescr::IDField;
using detdescr::DetectorID;

int main(int, const char* argv[])  {

    std::cout << "Hello Detector ID test!" << std::endl;

    IDField::IDFieldList fieldList;
    fieldList.push_back(new IDField("subdet", 0, 0, 3));
    fieldList.push_back(new IDField("layer", 1, 4, 11));
    DetectorID* detID = new DetectorID(&fieldList);

    int layer = 10;
    int subdet = 2;

    detID->setFieldValue("layer", layer);
    detID->setFieldValue("subdet", subdet);
    detID->pack();

    DetectorID::RawValue rawVal = detID->getRawValue();
    std::bitset<32> rawValBits = rawVal;
    std::cout << "rawValBits: " << rawValBits << std::endl;

    detID->setRawValue(rawVal);
    const DetectorID::FieldValueList& fieldValues = detID->unpack();

    std::cout << "subdet: " << fieldValues[0] << std::endl;
    std::cout << "layer: " << fieldValues[1] << std::endl;

    if (fieldValues[0] != subdet) {
        throw std::runtime_error("Wrong value for subdet: " + subdet);
    }

    if (fieldValues[1] != layer) {
        throw std::runtime_error("Wrong value for layer: " + layer);
    }

    delete detID;

    std::cout << "Bye Detector ID test!" << std::endl;
}
