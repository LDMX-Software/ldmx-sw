
#ifndef DETDESCR_SIMSPECIALID_H
#define DETDESCR_SIMSPECIALID_H

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

/**
 * @class SimSpecialID
 * @brief Implements detector ids for special simulation-derived hits like
 * scoring planes
 */
class SimSpecialID : public DetectorID {
 public:
  /**
   * Encodes which of several possible special types this SimSpecial ID is
   */
  enum SimSpecialType { SCORING_PLANE = 1 };

  static const RawValue SUBTYPE_MASK{0xF};  // space for up to 15 subtypes
  static const RawValue SUBTYPE_SHIFT{22};
  static const RawValue SUBTYPE_DATA_MASK{0x3FFFFF};

  // scoring plane only
  static const RawValue PLANE_MASK{
      0xFFF};  // space for up to 4096 scoring planes
  static const RawValue PLANE_SHIFT{0};

  /**
   * Empty id (but not null!)
   */
  SimSpecialID() : DetectorID(SD_SIM_SPECIAL, 0) {}

  /**
   * Create from raw number
   */
  SimSpecialID(RawValue rawid) : DetectorID(rawid) {
    SUBDETECTORID_TEST("SimSpecialID", SD_SIM_SPECIAL);
  }

  /**
   * Create from a DetectorID, but check
   */
  SimSpecialID(const DetectorID id) : DetectorID(id) {
    SUBDETECTORID_TEST("SimSpecialID", SD_SIM_SPECIAL);
  }

  /**
   * Create from a subtype number and raw field
   */
  SimSpecialID(SimSpecialType sst, RawValue rawfield)
      : DetectorID(SD_SIM_SPECIAL, 0) {
    id_ |= ((sst & SUBTYPE_MASK) << SUBTYPE_SHIFT);
    id_ |= rawfield & SUBTYPE_DATA_MASK;
  }

  /**
   * Create a scoring id from pieces
   */
  static SimSpecialID ScoringPlaneID(int plane) {
    return SimSpecialID(SCORING_PLANE, (plane & PLANE_MASK) << PLANE_SHIFT);
  }

  /*
   * Get the value of the 'section' field from the ID.
   * @return The value of the 'strip' field.
   */
  SimSpecialType getSubtype() const {
    return SimSpecialType((id_ >> SUBTYPE_SHIFT) & SUBTYPE_MASK);
  }

  /**
   * Get the value of the plane field from the ID, if it is a scoring plane.
   * @return The value of the plane field or -1 if not a scoring plane
   */
  int plane() const {
    return (getSubtype() == SCORING_PLANE) ? ((id_ >> PLANE_SHIFT) & PLANE_MASK)
                                           : (-1);
  }

  /**
   * Get the raw payload contents
   */
  RawValue subtypePayload() const { return id_ & SUBTYPE_DATA_MASK; }

  static void createInterpreters();
};
}  // namespace ldmx

std::ostream& operator<<(std::ostream&, const ldmx::SimSpecialID&);

#endif
