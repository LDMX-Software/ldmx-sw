
/**
 * @file TrackPropagator.h
 * @brief Class that propagates tracks to the ECAL face
 * @author Tamas Vami (UCSB)
 */

#ifndef ECAL_TRACKPROPAGATOR_H_
#define ECAL_TRACKPROPAGATOR_H_

// LDMX
#include "Tracking/Event/Track.h"

// C++
#include <cmath>

namespace ecal {

/**
 * @class TrackPropagator
 * @brief Class that propagates tracks to the ECAL face
 */
class TrackPropagator {
 public:
  TrackPropagator() = default;

  virtual ~TrackPropagator() = default;

 public:
  /**
   * Return a vector of parameters for a propagated recoil track
   * @param[in] tracks The track collection
   * @param[in] ts_type The track state type, i.e. tracks state at the ECAL face
   * @param[in] ts_title The track state title, most likely "ecal"
   * @returns Vector of parameters for a propagated recoil track
   */

  static std::vector<float> trackProp(const ldmx::Tracks& tracks,
                                      ldmx::TrackStateType ts_type,
                                      const std::string& ts_title);
};

}  // namespace ecal

#endif
