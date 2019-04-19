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
            
    TrackerVetoResult::TrackerVetoResult() : TObject() {}

    TrackerVetoResult::~TrackerVetoResult() {}

    void TrackerVetoResult::Copy(TObject& object) const { 
        TrackerVetoResult& result = static_cast<TrackerVetoResult&>(object);
        result.passesVeto_	   = passesVeto_;
    }

    void TrackerVetoResult::Clear(Option_t *option) { 
        passesVeto_ = false;
    }    

    void TrackerVetoResult::Print(Option_t *option) { 
        std::cout << "[ TrackerVetoResult ]: Passes veto : " 
                  << " Passes veto: " << passesVeto_ << std::endl;
    }
}
