#include "DetDescr/IDField.h"

namespace ldmx {

IDField::IDField(std::string fieldName, unsigned index, unsigned startBit,
                 unsigned endBit)
    : field_name_(fieldName),
      index_(index),
      start_bit_(startBit),
      end_bit_(endBit) {
  // Create bit mask for the field.
  bit_mask_ = IDField::createBitMask(startBit, endBit);
}

const std::string& IDField::getFieldName() { return field_name_; }

unsigned IDField::getIndex() { return index_; }

unsigned IDField::getStartBit() { return start_bit_; }

unsigned IDField::getEndBit() { return end_bit_; }

unsigned IDField::getBitMask() { return bit_mask_; }

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
