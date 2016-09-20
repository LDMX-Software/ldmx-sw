#include "DetDescr/IdField.h"
        
IdField::IdField(std::string theFieldName, unsigned theIndex, unsigned theStartBit, unsigned theEndBit)
    : fieldName(theFieldName), index(theIndex), startBit(theStartBit), endBit(theEndBit) {

    // Create bit mask for the field.
    bitMask = IdField::createBitMask(startBit, endBit);
}

const std::string& IdField::getFieldName() {
    return fieldName;
}

unsigned IdField::getIndex() {
    return index;
}

unsigned IdField::getStartBit() {
    return startBit;
}

unsigned IdField::getEndBit() {
    return endBit;
}

unsigned IdField::getBitMask() {
    return bitMask;
}

unsigned IdField::createBitMask(unsigned startBit, unsigned endBit) {
   unsigned mask = 0;
   for (int i = startBit; i <= endBit; i++) {
       mask |= 1 << i;
   }
   return mask;
}
