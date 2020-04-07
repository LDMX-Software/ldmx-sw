/**
 * @file TrackerVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        TrackerVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/TrackerVetoResult.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

ClassImp(ldmx::TrackerVetoResult)

namespace ldmx {
            
    TrackerVetoResult::TrackerVetoResult() {}

    TrackerVetoResult::~TrackerVetoResult() {}

    void TrackerVetoResult::Clear() { 
        passesVeto_ = false;
    }    

    void TrackerVetoResult::Print(std::ostream& o) const { 
        o << "TrackerVetoResult {Passes veto : " << passesVeto_ <<  "}";
    }
}
