/**
 * @file EcalAbstractID.h
 * @brief Class that serves as a parent for ECal detector IDs of various types
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef DETDESCR_ECALABSTRACTID_H_
#define DETDESCR_ECALABSTRACTID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

/**
 * @class EcalAbstractID
 * @brief Parent of precision and trigger EcalIDs
 */
class EcalAbstractID : public DetectorID {
 public:
  enum EcalCellType {
    PrecisionGlobal = 0,  // full-granularity cells, transformed to a uniform
                          // grid not alternating by layer
    PrecisionLocal = 1,   // full-granularity cells, labelled by their local
                          // coordinatate system as from DAQ
    TriggerCell = 2,      // trigger cells
    Special = 7           // common-mode, calibration cells, etc
  };

  static const RawValue CELL_TYPE_MASK{
      0x7};  // space for up to eight cell types
  static const RawValue CELL_TYPE_SHIFT{23};
  static const RawValue ECAL_PAYLOAD_MASK{0x007FFFFF};

  /**
   * Empty ECAL id (but not null!)
   */
  EcalAbstractID() : DetectorID(SD_ECAL, 0) {}

  /**
   * Create from raw number
   */
  EcalAbstractID(RawValue rawid) : DetectorID(rawid) {
    SUBDETECTORID_TEST("EcalAbstractID", SD_ECAL);
  }

  /**
   * Create from a DetectorID, but check
   */
  EcalAbstractID(const DetectorID id) : DetectorID(id) {
    SUBDETECTORID_TEST("EcalAbstractID", SD_ECAL);
  }

  /**
   * Create from pieces
   */
  EcalAbstractID(unsigned int cell_type, unsigned int payload)
      : DetectorID(SD_ECAL, 0) {
    id_ |= (cell_type & CELL_TYPE_MASK) << CELL_TYPE_SHIFT;
    id_ |= (payload & ECAL_PAYLOAD_MASK);
  }

  /**
   * Get the value of the cell field from the ID.
   * @return The value of the cell field.
   */
  int cell_type() const { return (id_ >> CELL_TYPE_SHIFT) & CELL_TYPE_MASK; }
};

}  // namespace ldmx

#endif
