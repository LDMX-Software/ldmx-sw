/**
 * @file HcalTriggerID.h
 * @brief Class that defines an HCal trigger cell detector ID
 * @author Christian Herwig, Fermi National Accelerator Laboratory
 */

#ifndef DETDESCR_HCALTRIGGERID_H_
#define DETDESCR_HCALTRIGGERID_H_

// LDMX
#include "DetDescr/HcalAbstractID.h"

namespace ldmx {

/**
 * @class HcalTriggerID
 * @brief Extension of DetectorID providing access to HCal trigger cell
 */
class HcalTriggerID : public HcalAbstractID {
 public:
  /**
   * Encodes the section of the HCal based on the 'section' field value.
   */
  enum HcalSection { BACK = 0, TOP = 1, BOTTOM = 2, LEFT = 4, RIGHT = 3 };

  static const RawValue END_MASK{0x2}; // space for 2 ends plus a combined TP
  static const RawValue END_SHIFT{20};
  static const RawValue SECTION_MASK{0x7};  // space for up to 7 sections
  static const RawValue SECTION_SHIFT{18};
  static const RawValue LAYER_MASK{0xFF};  // space for up to 255 layers
  static const RawValue LAYER_SHIFT{10};
  static const RawValue SUPERSTRIP_MASK{0xFF};  // space for 255 superstrips/layer
  static const RawValue SUPERSTRIP_SHIFT{0};

  /**
   * Empty HCAL trigger id (but not null!)
   */
  HcalTriggerID() : HcalAbstractID() {}

  /**
   * Create from raw number
   */
  HcalTriggerID(RawValue rawid) : HcalAbstractID(rawid) {
    if (!null() && bar_type() != Trigger) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create HcalTriggerID from mismatched Hcal bar_type " +
              std::to_string(bar_type()));
    }
  }

  /**
   * Create from a DetectorID, but check
   */
  HcalTriggerID(const HcalAbstractID id) : HcalAbstractID(id) {
    if (!null() && bar_type() != Trigger) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create HcalTriggerID from mismatched Hcal bar_type " +
              std::to_string(bar_type()));
    }
  }

  /**
   * Create from pieces
   */
  HcalTriggerID(unsigned int section, unsigned int layer, unsigned int superstrip,
                 unsigned int end)
      : HcalAbstractID(Trigger, 0) {
    id_ |= (section & SECTION_MASK) << SECTION_SHIFT;
    id_ |= (layer & LAYER_MASK) << LAYER_SHIFT;
    id_ |= (superstrip & SUPERSTRIP_MASK) << SUPERSTRIP_SHIFT;
    id_ |= (end & END_MASK) << END_SHIFT;
  }

  /*
   * Get the value of the 'section' field from the ID.
   * @return The value of the 'section' field.
   */
  int getSection() const { return (id_ >> SECTION_SHIFT) & SECTION_MASK; }

  /*
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
   * Get the value of the 'superstrip' field from the ID.
   * @return The value of 'superstrip' field.
   */
  int getSuperstrip() const { return (id_ >> SUPERSTRIP_SHIFT) & SUPERSTRIP_MASK; }

  /**
   * Get the value of the 'superstrip' field from the ID.
   * @return The value of 'superstrip' field.
   */
  int superstrip() const { return (id_ >> SUPERSTRIP_SHIFT) & SUPERSTRIP_MASK; }

  /**
   * Get the value of the 'end' field from the ID.
   * @return The value of the 'end' field.
   */
  int end() const { return (id_ >> END_SHIFT) & END_MASK; }

  /**
   * Get whether the 'end' field from the ID is negative.
   * @return True if the end of the strip is negative
   */
  bool isNegativeEnd() const { return end() == 1; }

  /**
   * Get whether the ID is the composite of two bar ends.
   * @return True if the ID corresponds to a composite TP.
   */
  bool isComposite() const { return end() == 2; }

  static void createInterpreters();
};
}  // namespace ldmx

std::ostream& operator<<(std::ostream&, const ldmx::HcalTriggerID&);

#endif
