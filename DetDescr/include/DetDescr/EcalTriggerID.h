/**
 * @file EcalTriggerID.h
 * @brief Class that defines an ECal trigger cell detector ID
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef DETDESCR_ECALTRIGGERDETECTORID_H_
#define DETDESCR_ECALTRIGGERDETECTORID_H_

// LDMX
#include "DetDescr/EcalAbstractID.h"

namespace ldmx {

/**
 * @class EcalTriggerID
 * @brief Extension of DetectorID providing access to ECal trigger cell
 * information
 */
class EcalTriggerID : public EcalAbstractID {
 public:
  static const RawValue LAYER_MASK{0x3F};  // space for up to 64 layers
  static const RawValue LAYER_SHIFT{12};
  static const RawValue MODULE_MASK{0x1F};  // space for up to 32 modules/layer
  static const RawValue MODULE_SHIFT{7};
  static const RawValue CELL_MASK{
      0x7F};  // space for 128 trigger cells/module (!)
  static const RawValue CELL_SHIFT{0};

  /**
   * Empty ECAL id (but not null!)
   */
  EcalTriggerID() : EcalAbstractID() {}

  /**
   * Create from raw number
   */
  EcalTriggerID(RawValue rawid) : EcalAbstractID(rawid) {
    if (!null() && cell_type() != TriggerCell) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create EcalTriggerID from mismatched Ecal cell_type " +
              std::to_string(cell_type()));
    }
  }

  /**
   * Create from a DetectorID, but check
   */
  EcalTriggerID(const DetectorID id) : EcalAbstractID(id) {
    if (!null() && cell_type() != TriggerCell) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create EcalTriggerID from mismatched Ecal cell_type " +
              std::to_string(cell_type()));
    }
  }

  /**
   * Create from pieces
   */
  EcalTriggerID(unsigned int layer, unsigned int module, unsigned int cell)
      : EcalAbstractID(TriggerCell, 0) {
    id_ |= (layer & LAYER_MASK) << LAYER_SHIFT;
    id_ |= (module & MODULE_MASK) << MODULE_SHIFT;
    id_ |= (cell & CELL_MASK) << CELL_SHIFT;
  }

  /**
   * Get the value of the module field from the ID.
   * @return The value of the module field.
   */
  int module() const { return (id_ >> MODULE_SHIFT) & MODULE_MASK; }

  /**
   * Get the value of the module field from the ID.
   * @return The value of the module field.
   */
  int getModuleID() const { return (id_ >> MODULE_SHIFT) & MODULE_MASK; }

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
   * Get the value of the trigger cell field from the ID.
   * @return The value of the cell field.
   */
  int triggercell() const { return (id_ >> CELL_SHIFT) & CELL_MASK; }

  /**
   * Get the value of the trigger cell field from the ID.
   * @return The value of the cell field.
   */
  int getTriggerCellID() const { return (id_ >> CELL_SHIFT) & CELL_MASK; }

  /**
   * Get the cell u,v index assuming a CMS-standard 432-cell sensor
   * @return Pair providing a U/V index
   */
  std::pair<unsigned int, unsigned int> getCellUV() const;

  static void createInterpreters();
};

}  // namespace ldmx

std::ostream& operator<<(std::ostream&, const ldmx::EcalTriggerID&);

#endif
