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

    void TrackerVetoResult::Print() const { 
        std::cout << "[ TrackerVetoResult ]: Passes veto : " 
                  << " Passes veto: " << passesVeto_ << std::endl;
    }
}
