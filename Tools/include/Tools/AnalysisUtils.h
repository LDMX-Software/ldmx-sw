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
#include <vector>
#include <map>
#include <unordered_map>

namespace ldmx {

    // Forward declaration for classes inside ldmx namespace
    class FindableTrackResult; 
    class SimParticle;

    struct TrackMaps {
            std::unordered_map<int, const FindableTrackResult *> findable;        
            std::unordered_map<int, const FindableTrackResult *> loose;        
            std::unordered_map<int, const FindableTrackResult *> axial;        
    };

    namespace Analysis {

        /**
         * Find and return the sim particle associated with the recoil electron.
         *
         * @param particleMap map of sim particles
         * @return pointer to sim particle labeled as recoil electron (nullptr if not found)
         */
        const SimParticle* getRecoil(const std::map<int,SimParticle> &particleMap);

        /**
         * Get a pointer to the sim particle associated with the photon that
         * underwent a photo-nuclear reaction.
         *
         * Returns the first particle in the map that underwent a PN interaction.
         * If more than one particle undergoes a PN interaction, this will NOT notice.
         *
         * @param particleMap map of sim particles
         * @return pointer to sim particle labeled as PN Gamma photon (nullptr if not found)
         */
        const SimParticle* getPNGamma(const std::map<int,SimParticle> &particleMap);

        /**
         * Get a pointer to the sim particle associated with the photon daughter
         * of the recoil electron that underwent a photo-nuclear reaction.
         *
         * @param particleMap map of sim particles
         * @return pointer to sim particle (nullptr if not found)
         */
        const SimParticle* getRecoilPNGamma(const std::map<int,SimParticle> &particleMap);

        /**
         * Sort tracks depending on how finable they are.
         */
        TrackMaps getFindableTrackMaps(const std::vector<FindableTrackResult> &tracks);

    } // Analysis

} // ldmx

#endif // _ANALYSIS_UTILS_H_
