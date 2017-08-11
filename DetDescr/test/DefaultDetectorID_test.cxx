// LDMX
#include "DetDescr/DefaultDetectorID.h"

// STL
#include <bitset>
#include <iostream>
#include <stdexcept>

using ldmx::IDField;
using ldmx::DefaultDetectorID;
using ldmx::FieldValueList;
using ldmx::DetectorID;

int main(int, const char* argv[])  {

    std::cout << "Hello DefaultDetectorID test!" << std::endl;

    DetectorID* detID = new DefaultDetectorID();

    int layer = 10;
    int subdet = 2;

    detID->setFieldValue("layer", layer);
    detID->setFieldValue("subdet", subdet);
    detID->pack();

    DetectorID::RawValue rawVal = detID->getRawValue();
    std::bitset<32> rawValBits = rawVal;
    std::cout << "rawValBits: " << rawValBits << std::endl;

    detID->setRawValue(rawVal);
    const FieldValueList& fieldValues = detID->unpack();

    std::cout << "subdet: " << fieldValues[0] << std::endl;
    std::cout << "layer: " << fieldValues[1] << std::endl;

    if (fieldValues[0] != subdet) {
        throw std::runtime_error("Wrong value for subdet: " + subdet);
    }

    if (fieldValues[1] != layer) {
        throw std::runtime_error("Wrong value for layer: " + layer);
    }

    delete detID;

    std::cout << "Bye DefaultDetectorID test!" << std::endl;
}
