/**
 * @file HcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        HcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/HcalVetoResult.h"

ClassImp(ldmx::HcalVetoResult)

namespace ldmx {
            
    HcalVetoResult::HcalVetoResult() :
        TObject() {  
    }

    HcalVetoResult::~HcalVetoResult() {
    }

    void HcalVetoResult::Copy(TObject& object) const { 
        HcalVetoResult& result = (HcalVetoResult&) object;
        result.passesVeto_	    	= passesVeto_;
    }

    void HcalVetoResult::Clear(Option_t *option) { 
        passesVeto_ = false; 
    }

    void HcalVetoResult::Print(Option_t *option) { 
        std::cout << "[ HcalVetoResult ]: Passes veto : " << passesVeto_ << std::endl;
    }
}
