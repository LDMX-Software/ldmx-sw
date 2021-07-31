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

  static const RawValue SECTION_MASK{0x7};  // space for up to 7 sections
  static const RawValue SECTION_SHIFT{18};
  static const RawValue LAYER_MASK{0xFF};  // space for up to 255 layers
  static const RawValue LAYER_SHIFT{10};
  static const RawValue STRIP_MASK{0xFF};  // space for 255 strips/layer
  static const RawValue STRIP_SHIFT{0};

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
  HcalTriggerID(unsigned int section, unsigned int layer, unsigned int strip)
      : HcalAbstractID(Trigger, 0) {
    id_ |= (section & SECTION_MASK) << SECTION_SHIFT;
    id_ |= (layer & LAYER_MASK) << LAYER_SHIFT;
    id_ |= (strip & STRIP_MASK) << STRIP_SHIFT;
  }

  /*
   * Get the value of the 'section' field from the ID.
   * @return The value of the 'strip' field.
   */
  int getSection() const { return (id_ >> SECTION_SHIFT) & SECTION_MASK; }

  /*
   * Get the value of the 'section' field from the ID.
   * @return The value of the 'strip' field.
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
  int getSuperstrip() const { return (id_ >> STRIP_SHIFT) & STRIP_MASK; }

  /**
   * Get the value of the 'superstrip' field from the ID.
   * @return The value of 'superstrip' field.
   */
  int superstrip() const { return (id_ >> STRIP_SHIFT) & STRIP_MASK; }
  
  static void createInterpreters();
};
}  // namespace ldmx

std::ostream& operator<<(std::ostream&, const ldmx::HcalTriggerID&);

#endif
