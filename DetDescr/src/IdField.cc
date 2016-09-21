#include "DetDescr/IdField.h"
        
IdField::IdField(std::string fieldName, unsigned index, unsigned startBit, unsigned endBit)
    : fieldName(fieldName), index(index), startBit(startBit), endBit(endBit) {

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
