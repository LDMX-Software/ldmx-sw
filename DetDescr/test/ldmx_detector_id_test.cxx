// LDMX
#include "DetDescr/DetectorID.h"
#include "Exception/Exception.h"

// STL
#include <bitset>
#include <iostream>
#include <stdexcept>

using ldmx::IDField;
using ldmx::DetectorID;

int main(int, const char* argv[])  {

    try{
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
            EXCEPTION_RAISE( "IDTest" , "Wrong value for subdet: " + std::to_string(subdet) );
        }
    
        if (fieldValues[1] != layer) {
            EXCEPTION_RAISE( "IDTest" , "Wrong value for layer: " + std::to_string(layer) );
        }
    
        delete detID;
    
        std::cout << "Bye Detector ID test!" << std::endl;
    } catch (ldmx::Exception& e) {
        std::cerr << "DetID Error [" << e.name() << "] : " << e.message() << std::endl;
        std::cerr << "  at " << e.module() << ":" << e.line() << " in " << e.function() << std::endl;
    }
}
