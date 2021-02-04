/**
 * @file EcalID.h
 * @brief Class that defines an ECal detector ID with a cell number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_ECALDETECTORID_H_
#define DETDESCR_ECALDETECTORID_H_

// LDMX
#include "DetDescr/EcalAbstractID.h"

namespace ldmx {

/**
 * @class EcalID
 * @brief Extension of DetectorID providing access to ECal layers and cell
 * numbers in a hex grid
 */
class EcalID : public EcalAbstractID {
 public:
  static const RawValue LAYER_MASK{0x3F};  // space for up to 64 layers
  static const RawValue LAYER_SHIFT{17};
  static const RawValue MODULE_MASK{0x1F};  // space for up to 32 modules/layer
  static const RawValue MODULE_SHIFT{12};
  static const RawValue CELL_MASK{0xFFF};  // space for 4096 cells/module (!)
  static const RawValue CELL_SHIFT{0};

  /**
   * Empty ECAL id (but not null!)
   */
  EcalID() : EcalAbstractID() {}

  /**
   * Create from raw number
   */
  EcalID(RawValue rawid) : EcalAbstractID(rawid) {
    if (!null() && cell_type() != PrecisionGlobal &&
        cell_type() != PrecisionLocal) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create EcalID from mismatched Ecal cell_type " +
              std::to_string(cell_type()));
    }
  }

  /**
   * Create from a DetectorID, but check
   */
  EcalID(const DetectorID id) : EcalAbstractID(id) {
    if (!null() && cell_type() != PrecisionGlobal &&
        cell_type() != PrecisionLocal) {
      EXCEPTION_RAISE(
          "DetectorIDMismatch",
          "Attempted to create EcalID from mismatched Ecal cell_type " +
              std::to_string(cell_type()));
    }
  }

  /**
   * Create from pieces
   */
  EcalID(unsigned int layer, unsigned int module, unsigned int cell)
      : EcalAbstractID(PrecisionGlobal, 0) {
    id_ |= (layer & LAYER_MASK) << LAYER_SHIFT;
    id_ |= (module & MODULE_MASK) << MODULE_SHIFT;
    id_ |= (cell & CELL_MASK) << CELL_SHIFT;
  }

  /**
   * Create from pieces including u/v cell
   */
  EcalID(unsigned int layer, unsigned int module, unsigned int u,
         unsigned int v);

  /**
   * Create from pieces including u/v cell
   */
  EcalID(unsigned int layer, unsigned int module,
         std::pair<unsigned int, unsigned int> uv)
      : EcalID(layer, module, uv.first, uv.second) {}

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
   * Get the value of the cell field from the ID.
   * @return The value of the cell field.
   */
  int cell() const { return (id_ >> CELL_SHIFT) & CELL_MASK; }

  /**
   * Get the value of the cell field from the ID.
   * @return The value of the cell field.
   */
  int getCellID() const { return (id_ >> CELL_SHIFT) & CELL_MASK; }

  /**
   * Get the cell u,v index assuming a CMS-standard 432-cell sensor
   * @return Pair providing a U/V index
   */
  std::pair<unsigned int, unsigned int> getCellUV() const;

  static void createInterpreters();
};

}  // namespace ldmx

std::ostream& operator<<(std::ostream&, const ldmx::EcalID&);

#endif
