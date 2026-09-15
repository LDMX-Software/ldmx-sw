
/**
 * @file EcalHelper.h
 * @brief Class that propagates tracks to the ECAL face
 * @author Tamas Vami, Jihoon Yoo (UCSB)
 */

#ifndef ECAL_TRACKPROPAGATOR_H_
#define ECAL_TRACKPROPAGATOR_H_

// LDMX
#include "Tracking/Event/Track.h"

// C++
#include <cmath>

// ROOT
#include "Math/Vector3D.h"

namespace ecal {

/**
 * Return a vector of parameters for a propagated recoil track
 * @param[in] tracks The track collection
 * @param[in] ts_type The track state type, i.e. tracks state at the ECAL face
 * @param[in] ts_title The track state title, most likely "ecal"
 * @returns Vector of parameters for a propagated recoil track
 */

std::vector<float> trackProp(const ldmx::Tracks& tracks,
                             ldmx::TrackStateType ts_type,
                             const std::string& ts_title);

/**
 * Return a vector of `ele_count` valid track states with the greatest
 * transverse momentum
 * @param[in] tracks The track collection
 * @param[in] ele_count The recorded number of electrons in the event
 * @returns Vector of valid track states with the greatest transverse momentum
 */
std::vector<std::vector<float>> pTTrackProp(const ldmx::Tracks& tracks,
                                            int ele_count);

/**
 * Return a vector of `ele_count` valid track states with the greatest total
 * momentum
 * @param[in] tracks The track collection
 * @param[in] ele_count The recorded number of electrons in the event
 * @returns Vector of valid track states with the greatest total momentum
 */
std::vector<std::vector<float>> momTrackProp(const ldmx::Tracks& tracks,
                                             int ele_count);

// MIP tracking
/**
 * Returns the distance between the lines v and w, with v defined to pass
 * through the points (v1,v2) (and similarly for w).
 *
 * @param[in] v1 An arbitrary point on line v
 * @param[in] v2 A second, distinct point on line v
 * @param[in] w1 An arbitrary point on line w
 * @param[in] w2 A second, distinct point on line w
 * @returns Closest distance of approach of lines u and v
 */
float distTwoLines(ROOT::Math::XYZVector v1, ROOT::Math::XYZVector v2,
                   ROOT::Math::XYZVector w1, ROOT::Math::XYZVector w2);
/**
 * Return the minimum distance between the point h1 and the line passing
 * through points p1 and p2.
 *
 * @param[in] h1 Point to find the distance to
 * @param[in] p1 An arbitrary point on the line
 * @param[in] p2 A second, distinct point on the line
 * @returns Minimum distance between h1 and the line
 */
float distPtToLine(ROOT::Math::XYZVector h1, ROOT::Math::XYZVector p1,
                   ROOT::Math::XYZVector p2);

}  // namespace ecal

#endif
