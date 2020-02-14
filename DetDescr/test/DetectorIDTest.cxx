/**
 * @file DetectorIDTest.cxx
 * @brief Test the operation of DetectorID class
 */
#include "Exception/catch.hpp"

#include "DetDescr/DetectorID.h"

TEST_CASE( "Detector ID Test" , "[function]" ) {

    using ldmx::IDField;
    using ldmx::DetectorID;
    
    IDField::IDFieldList *fieldList = new IDField::IDFieldList();
    fieldList->push_back(new IDField("subdet", 0, 0, 3));
    fieldList->push_back(new IDField("layer", 1, 4, 11));
    DetectorID detID(fieldList); //DetectorID owns fieldList and its members

    int layer = 10;
    int subdet = 2;

    detID.setFieldValue("layer", layer);
    detID.setFieldValue("subdet", subdet);
    detID.pack();

    DetectorID::RawValue rawVal = detID.getRawValue();

    detID.setRawValue(rawVal);
    const DetectorID::FieldValueList& fieldValues = detID.unpack();

    REQUIRE( fieldValues[0] == subdet );
    REQUIRE( detID.getFieldValue( "subdet" ) == subdet );
    REQUIRE( fieldValues[1] == layer  );
    REQUIRE( detID.getFieldValue( "layer" ) == layer );

}
