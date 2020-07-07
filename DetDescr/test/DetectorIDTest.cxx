/**
 * @file DetectorIDTest.cxx
 * @brief Test the operation of DetectorID class
 */
#include "Framework/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "DetDescr/DetectorID.h" //headers defining what we will be testing
#include "DetDescr/DefaultDetectorID.h" //headers defining what we will be testing

/**
 * Test for DetectorID function
 *
 * First argument is name of this test.
 * Second argument is tags to group tests together.
 */
TEST_CASE( "Detector ID Test" , "[DetDescr][functionality]" ) {

    using ldmx::IDField;
    using ldmx::DetectorID;
    using ldmx::DefaultDetectorID;
    
    SECTION( "General Detector ID" ) {
        IDField::IDFieldList *fieldList = new IDField::IDFieldList();
        fieldList->push_back(new IDField("subdet", 0, 0, 3));
        fieldList->push_back(new IDField("layer", 1, 4, 11));
        DetectorID detID(fieldList); //DetectorID owns fieldList and its members
    
        int layer = 10;
        int subdet = 2;
    
        detID.setFieldValue("layer", layer);
        detID.setFieldValue("subdet", subdet);
        detID.pack(); //needs to happen to save new values
    
        DetectorID::RawValue rawVal = detID.getRawValue();
    
        detID.setRawValue(rawVal);
        const DetectorID::FieldValueList& fieldValues = detID.unpack();
    
        REQUIRE( fieldValues[0] == subdet );
        REQUIRE( detID.getFieldValue( "subdet" ) == subdet );
        REQUIRE( fieldValues[1] == layer  );
        REQUIRE( detID.getFieldValue( "layer" ) == layer );
    }

    SECTION( "Default Detector ID" ) {

        DefaultDetectorID defaultID; //default ID handles IDFields internally

        int layer = 10;
        int subdet = 2;
    
        defaultID.setFieldValue( "layer" , layer );
        defaultID.setFieldValue( "subdet" , subdet );
        defaultID.pack(); //needs to happen to save new values
    
        REQUIRE( defaultID.getSubdetID() == subdet );
        REQUIRE( defaultID.getLayerID()  == layer  );

    }
}
