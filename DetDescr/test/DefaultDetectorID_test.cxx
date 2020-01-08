// LDMX
#include "DetDescr/DefaultDetectorID.h"
#include "Exception/Exception.h"

// STL
#include <bitset>
#include <iostream>

using ldmx::IDField;
using ldmx::DefaultDetectorID;
using ldmx::DetectorID;

int main(int, const char* argv[])  {

    try{
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
        const DetectorID::FieldValueList& fieldValues = detID->unpack();
    
        std::cout << "subdet from field values: " << fieldValues[0] << std::endl;
        std::cout << "layer from field values: " << fieldValues[1] << std::endl;
    
        /*
         * Check values from decoded list.
         */
        if (fieldValues[0] != subdet) {
            EXCEPTION_RAISE( "IDTest" , "Wrong value for subdet: " + std::to_string(fieldValues[0]) );
        } else {
            std::cout << "subdet field value okay" << std::endl;
        }
    
        if (fieldValues[1] != layer) {
            EXCEPTION_RAISE( "IDTest" , "Wrong value for layer: " + std::to_string(fieldValues[1]) );
        } else {
            std::cout << "layer field value okay" << std::endl;
        }
    
        /*
         * Check individually decoded values from indices.
         */
        if (detID->getFieldValue(0) != subdet) {
            EXCEPTION_RAISE( "IDTest" , "Wrong value for subdet: " + std::to_string(detID->getFieldValue(0)) );
        } else {
            std::cout << "subdet from getFieldValue(i) okay" << std::endl;
        }
    
        if (detID->getFieldValue(1) != layer) {
            EXCEPTION_RAISE( "IDTest" , "Wrong value for layer: " + std::to_string(detID->getFieldValue(1)) );
        } else {
            std::cout << "layer from getFieldValue(i) okay" << std::endl;
        }
    
        /*
         * Check individually decoded values from field names.
         */
        if (detID->getFieldValue("subdet") != subdet) {
            EXCEPTION_RAISE( "IDTest" , "Wrong value for subdet: " + std::to_string(detID->getFieldValue("subdet")) );
        } else {
            std::cout << "subdet from getFieldValue(name) okay" << std::endl;
        }
    
        if (detID->getFieldValue("layer") != layer) {
            EXCEPTION_RAISE( "IDTest" , "Wrong value for layer: " + std::to_string(detID->getFieldValue("layer")) );
        } else {
            std::cout << "layer from getFieldValue(name) okay" << std::endl;
        }
    
        std::cout << "Bye DefaultDetectorID test!" << std::endl;

    } catch( ldmx::Exception& e ) {
        std::cerr << "DefaultIDTest Error [" << e.name() << "] : " << e.message() << std::endl;
        std::cerr << "  at " << e.module() << ":" << e.line() << " in " << e.function() << std::endl;
    }
}
