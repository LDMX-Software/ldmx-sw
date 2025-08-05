/**
 * @file TrackerID.h
 * @brief Class that defines a Tracker detector ID with a module_ number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_TRACKERID_H_
#define DETDESCR_TRACKERID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

/**
 * @class TrackerID
 * @brief Extension of DetectorID providing access to layer_ and module_ number
 * for tracker IDs
 */
class TrackerID : public DetectorID {
 public:
  static const RawValue LAYER_MASK{0xFF};
  static const RawValue LAYER_SHIFT{0};
  static const RawValue MODULE_MASK{0x1F};
  static const RawValue MODULE_SHIFT{8};

  /**
   * Create a null TrackerID (not useful)
   */
  TrackerID() {}

  /**
   * Create from a DetectorID, but check
   */
  TrackerID(const DetectorID id) : DetectorID(id) {
    SUBDETECTORID_TEST2("TrackerID", SD_TRACKER_TAGGER, SD_TRACKER_RECOIL);
  }

  /**
   * Create from a raw id, but check
   */
  TrackerID(RawValue rawid) : DetectorID(rawid) {
    SUBDETECTORID_TEST2("TrackerID", SD_TRACKER_TAGGER, SD_TRACKER_RECOIL);
  }

  /** Create from values
   */
  TrackerID(SubdetectorIDType system, unsigned int layer_, unsigned int module_)
      : DetectorID(system, 0) {
    id_ |= (layer_ & LAYER_MASK) << LAYER_SHIFT;
    id_ |= (module_ & MODULE_MASK) << MODULE_SHIFT;
  }

  /**
   * Get the value of the module_ field from the ID.
   * @return The value of the module_ field.
   */
  int module() const { return (id_ >> MODULE_SHIFT) & MODULE_MASK; }

  /**
   * Get the value of the layer_ field from the ID.
   * @return The value of the layer_ field.
   */
  int layer() const { return (id_ >> LAYER_SHIFT) & LAYER_MASK; }

  friend std::ostream& operator<<(std::ostream& o, const ldmx::TrackerID& d);

  static void createInterpreters();
};
}  // namespace ldmx

#endif
