/**
 * @file HcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        HcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/HcalVetoResult.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/HcalHit.h"

ClassImp(ldmx::HcalVetoResult)

namespace ldmx {
            
    HcalVetoResult::HcalVetoResult() {}

    HcalVetoResult::~HcalVetoResult() {}

    void HcalVetoResult::Clear() { 
        passesVeto_ = false;
    }

    void HcalVetoResult::Print(std::ostream& o) const { 
        o << "HcalVetoResult {Passes veto : " << std::boolalpha << passesVeto_ << "}";
    }
}
