
/**
 * @file EcalVetoResult.cxx
 * @brief Class used to encapsulate the results obtained from 
 *        EcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/EcalVetoResult.h"

ClassImp(ldmx::EcalVetoResult)

namespace ldmx {
            
    EcalVetoResult::EcalVetoResult() :
        TObject() {  
    }

    EcalVetoResult::~EcalVetoResult() {
    }

    void EcalVetoResult::setResult(bool passesVeto, float summedDep, float summedIso, float backSummedDep) { 
        passesVeto_ = passesVeto; 
        summedDep_  = summedDep; 
        summedIso_  = summedIso; 
        backSummedDep_ = backSummedDep; 
    }

    void EcalVetoResult::Clear(Option_t *option) { 
        passesVeto_ = false; 
        summedDep_      = 0;
        summedIso_      = 0;  
        backSummedDep_  = 0;  
    }

    void EcalVetoResult::Print(Option_t *option) { 
        std::cout << "[ EcalVetoResult ]:\n" 
          << "\t Passes veto : " << passesVeto_ << "\n"
          << "\t summedDep: " << summedDep_ << "\n"
          << "\t summedIso: " << summedIso_ << "\n"
          << "\t backSummedDep: " << backSummedDep_ << "\n"
          << std::endl;
    }
}
