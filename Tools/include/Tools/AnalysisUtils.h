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
#include <unordered_map>

// Forward declaration
class TClonesArray; 

namespace ldmx {

    // Forward declaration for classes inside ldmx namespace
    class FindableTrackResult; 
    class SimParticle;

    struct TrackMaps {
            std::unordered_map<const SimParticle*, const FindableTrackResult*> findable;        
            std::unordered_map<const SimParticle*, const FindableTrackResult*> loose;        
            std::unordered_map<const SimParticle*, const FindableTrackResult*> axial;        
    };

    namespace Analysis {

        /**
         * Find and return the the sim particle associated with the recoil 
         * electron.
         *
         * @param particles Collection of sim particles
         * @param index Position along the TClonesArray to start the search for 
         *              the recoil.  The default is to start at the 
         *              beginning of the array. 
         */
        const SimParticle* searchForRecoil(const TClonesArray* particles, const int index = 0);

        /**
         * Find and return the sim particle associated with the gamma that 
         * underwent a photo-nuclear reaction.
         *
         * @param particle Sim particle that will be used to retrieve the list of 
         *                 particles to search through.
         * @param index Position along the TClonesArray to start the search for the
         *              photo-nuclear gamma.  The default is to start at the 
         *              beginning of the array.
         */
        const SimParticle* searchForPNGamma(const SimParticle* particle, const int index = 0);  

        /**
         * 
         */
        TrackMaps getFindableTrackMaps(const TClonesArray* tracks);

        void printDaughters(const SimParticle* particle, std::string prefix = ""); 


    } // Analysis

} // ldmx

#endif // _ANALYSIS_UTILS_H_
