#include <iostream>

// LDMX
#include <bitset>
#include <iostream>
#include "../include/DetDescr/DetectorID.h"

int main(int, const char* argv[])  {

    std::cout << "Hello Detector ID test!" << std::endl;

    IDField::IDFieldList fieldList;
    fieldList.push_back(new IDField("subdet", 0 /* index */, 0 /* start bit */, 3 /* end bit */));
    fieldList.push_back(new IDField("layer", 1, 4, 11));

    DetectorID* detId = new DetectorID(&fieldList);

    IDField* field1 = fieldList[0];
    std::bitset<32> bitMask1 = field1->getBitMask();
    std::cout << field1->getFieldName() << " bit mask = " << bitMask1 << std::endl;

    IDField* field2 = fieldList[1];
    std::bitset<32> bitMask2 = field2->getBitMask();
    std::cout << field2->getFieldName() << " bit mask = " << bitMask2 << std::endl;

    unsigned subdet = 1;
    unsigned layer = 10;

    DetectorID::RawValue val = subdet | (layer << 4);
    detId->setRawValue(val);

    std::bitset<32> valBits = val;
    std::cout << "raw val bits = " << valBits << std::endl;

    DetectorID::FieldValueList unpacked = detId->unpack();
    std::cout << "val[0] = " << unpacked[0] << std::endl;
    std::cout << "val[1] = " << unpacked[1] << std::endl;

    DetectorID::RawValue packed = detId->pack();
    std::bitset<32> packedBits = packed;
    std::cout << "packed val bits = " << packedBits << std::endl;

    delete detId;

    std::cout << "Bye Detector ID test!" << std::endl;
}
