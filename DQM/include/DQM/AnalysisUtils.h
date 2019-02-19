/**
 * @file AnalysisUtils.h
 * @brief Collection of utility functions useful for analysis
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _ANALYSIS_UTILS_H_
#define _ANALYSIS_UTILS_H_


// Forward declaration
class TClonesArray;  

namespace ldmx { 

    // Forward declaration for classes inside ldmx namespace
    class SimParticle;

    /**
     * Find and return the the sim particle associated with the recoil 
     * electron.
     *
     * @param particles Collection of sim particles
     * @param index Position along the TClonesArray to start the search for 
     *              the recoil.  The default is to search start at the 
     *              beginning of the array. 
     */
    const SimParticle* searchForRecoil(const TClonesArray* particles, const int index = 0); 

} // ldmx

#endif // _ANALYSIS_UTILS_H_
