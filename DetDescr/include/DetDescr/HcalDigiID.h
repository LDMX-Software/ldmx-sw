/**
 * @file HcalDigiID.h
 * @brief Class that defines an HCal digi detector ID
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#ifndef DETDESCR_HCALDIGIID_H_
#define DETDESCR_HCALDIGIID_H_

// LDMX
#include "DetDescr/HcalAbstractID.h"

namespace ldmx {

/**
 * @class HcalDigiID
 * @brief Extension of HcalAbstractID providing access to HCal digi information
 */
class HcalDigiID : public HcalAbstractID {
 public:
  static const RawValue END_MASK{0x1};  // space for up to 2 ends of a strip
  static const RawValue END_SHIFT{19};
  static const RawValue SECTION_MASK{0x7};  // space for up to 7 sections
  static const RawValue SECTION_SHIFT{16};
  static const RawValue LAYER_MASK{0xFF};  // space for up to 255 layers
  static const RawValue LAYER_SHIFT{8};
  static const RawValue STRIP_MASK{0xFF};  // space for 255 strips/layer
  static const RawValue STRIP_SHIFT{0};

  /**
   * Empty HCAL id (but not null!)
   */
  HcalDigiID() : HcalAbstractID() {}

  /**
   * Create from raw number
   */
  HcalDigiID(RawValue rawid) : HcalAbstractID(rawid) {
    if (!null() && bar_type() != Digi) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create HcalID from mismatched Hcal bar_type " +
              std::to_string(bar_type()));
    }
  }

  /**
   * Create from a DetectorID, but check
   */
  HcalDigiID(const DetectorID id) : HcalAbstractID(id) {
    if (!null() && bar_type() != Digi) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create HcalID from mismatched Hcal bar_type " +
              std::to_string(bar_type()));
    }
  }

  /**
   * Create from pieces
   */
  HcalDigiID(unsigned int section, unsigned int layer, unsigned int strip,
             unsigned int end)
      : HcalAbstractID(Digi, 0) {
    id_ |= (section & SECTION_MASK) << SECTION_SHIFT;
    id_ |= (layer & LAYER_MASK) << LAYER_SHIFT;
    id_ |= (strip & STRIP_MASK) << STRIP_SHIFT;
    id_ |= (end & END_MASK) << END_SHIFT;
  }

  /**
   * Get the value of the 'section' field from the ID.
   * @return The value of the 'section' field.
   */
  int getSection() const { return (id_ >> SECTION_SHIFT) & SECTION_MASK; }

  /**
   * Get the value of the 'section' field from the ID.
   * @return The value of the 'section' field.
   */
  int section() const { return (id_ >> SECTION_SHIFT) & SECTION_MASK; }

  /**
   * Get the value of the layer field from the ID.
   * @return The value of the layer field.
   */
  int layer() const { return (id_ >> LAYER_SHIFT) & LAYER_MASK; }

  /**
   * Get the value of the layer field from the ID.
   * @return The value of the layer field.
   */
  int getLayerID() const { return (id_ >> LAYER_SHIFT) & LAYER_MASK; }

  /**
   * Get the value of the 'strip' field from the ID.
   * @return The value of 'strip' field.
   */
  int getStrip() const { return (id_ >> STRIP_SHIFT) & STRIP_MASK; }

  /**
   * Get the value of the 'strip' field from the ID.
   * @return The value of 'strip' field.
   */
  int strip() const { return (id_ >> STRIP_SHIFT) & STRIP_MASK; }

  /**
   * Get the value of the 'end' field from the ID.
   * @return The value of the 'end' field.
   */
  int end() const { return (id_ >> END_SHIFT) & END_MASK; }

  /**
   * Get whether the 'end' field from the ID is negative.
   * @return True if the end of the strip is negative
   */
  bool isNegativeEnd() const {
    if (end() == 1)
      return true;
    else
      return false;
  }

  static void createInterpreters();
};
}  // namespace ldmx

std::ostream& operator<<(std::ostream&, const ldmx::HcalDigiID&);

#endif
