#include "DetDescr/IDField.h"

namespace ldmx {

IDField::IDField(std::string fieldName, unsigned index, unsigned startBit,
                 unsigned endBit)
    : fieldName_(fieldName),
      index_(index),
      startBit_(startBit),
      endBit_(endBit) {
  // Create bit mask for the field.
  bitMask_ = IDField::createBitMask(startBit, endBit);
}

const std::string& IDField::getFieldName() { return fieldName_; }

unsigned IDField::getIndex() { return index_; }

unsigned IDField::getStartBit() { return startBit_; }

unsigned IDField::getEndBit() { return endBit_; }

unsigned IDField::getBitMask() { return bitMask_; }

unsigned IDField::createBitMask(unsigned startBit, unsigned endBit) {
  unsigned mask = 0;
  for (int i = startBit; i <= endBit; i++) {
    mask |= 1 << i;
  }
  return mask;
}

unsigned IDField::countOnes(unsigned mask) {
  unsigned rv = 0;
  for (int i = 0; i < 32; i++)
    if (mask & (1 << i)) rv++;
  return rv;
}

}  // namespace ldmx
