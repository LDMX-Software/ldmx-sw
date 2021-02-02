/**
 * @file AnalysisUtils.h
 * @brief Collection of utility functions useful for analysis
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _ANALYSIS_UTILS_H_
#define _ANALYSIS_UTILS_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <cmath>
#include <map>
#include <unordered_map>
#include <vector>

// Forward declaration for classes inside ldmx namespace
// class FindableTrackResult;
namespace ldmx {
class SimParticle;
}

/*
struct TrackMaps {
  std::unordered_map<int, const FindableTrackResult *> findable;
  std::unordered_map<int, const FindableTrackResult *> loose;
  std::unordered_map<int, const FindableTrackResult *> axial;
};*/

namespace Analysis {

/**
 * Find and return the sim particle associated with the recoil electron.
 *
 * @param[in] particleMap map of sim particles
 *
 * @return[out] Pointer to sim particle labeled as recoil electron (nullptr if
 * not found)
 */
std::tuple<int, const ldmx::SimParticle *> getRecoil(
    const std::map<int, ldmx::SimParticle> &particleMap);

/**
 * Get a pointer to the sim particle associated with the photon that
 * underwent a photo-nuclear reaction.
 *
 * Returns the first particle in the map that underwent a PN interaction
 * and has an energy above the threshold. If more than one particle
 * satisfied this condition, this will NOT notice.
 *
 * @param[in] particleMap Map of sim particles
 * @param[in] recoil The recoil electron
 * @param[in] energyThreshold The energy that the photon energy must be
 *      greater than.
 *
 * @return[out] Pointer to sim particle labeled as PN Gamma photon (nullptr if
 * not found)
 */
const ldmx::SimParticle *getPNGamma(
    const std::map<int, ldmx::SimParticle> &particleMap,
    const ldmx::SimParticle *recoil, const float &energyThreshold);

/**
 * Sort tracks depending on how finable they are.
 */
// TrackMaps getFindableTrackMaps(const std::vector<FindableTrackResult>
// &tracks);

}  // namespace Analysis

#endif  // _ANALYSIS_UTILS_H_
