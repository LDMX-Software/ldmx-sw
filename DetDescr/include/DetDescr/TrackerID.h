/**
 * @file TrackerID.h
 * @brief Class that defines a Tracker detector ID with a module number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_TRACKERID_H_
#define DETDESCR_TRACKERID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

/**
 * @class TrackerID
 * @brief Extension of DetectorID providing access to layer and module number
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
  TrackerID(SubdetectorIDType system, unsigned int layer, unsigned int module)
      : DetectorID(system, 0) {
    id_ |= (layer & LAYER_MASK) << LAYER_SHIFT;
    id_ |= (module & MODULE_MASK) << MODULE_SHIFT;
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
  int layer() const { return (id_ >> LAYER_SHIFT) & LAYER_MASK; }

  static void createInterpreters();
};
}  // namespace ldmx

std::ostream& operator<<(std::ostream&, const ldmx::TrackerID&);

#endif
