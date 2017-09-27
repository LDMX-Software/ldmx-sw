/**
 * @file PnWeightResult.cxx
 * @brief Class used to encapsulate the results obtained from
 *        PnWeightProcessor.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/PnWeightResult.h"

ClassImp(ldmx::PnWeightResult)

namespace ldmx {

    PnWeightResult::PnWeightResult() :
        TObject() {
    }

    PnWeightResult::~PnWeightResult() {
        Clear();
    }

    void PnWeightResult::Clear(Option_t *option) {
        TObject::Clear();

        hardestNucleonKe_ = -9999;
        hardestNucleonTheta_ = -9999;
        hardestNucleonW_ = -9999; 
        weight_ = 1.0; 
    }

    void PnWeightResult::Copy(TObject& object) const { 
        PnWeightResult& result = (PnWeightResult&) object; 


        result.hardestNucleonKe_ = hardestNucleonKe_;
        result.hardestNucleonTheta_ = hardestNucleonTheta_;
        result.hardestNucleonW_ = hardestNucleonW_; 
        result.weight_ = weight_; 

    }

    void PnWeightResult::Print(Option_t *option) const {
        std::cout << "[ PnWeightResult ]:\n"
                  << "\t Hardest nucleon KE: " << hardestNucleonKe_ << "\n"
                  << "\t Hardest nucleon theta: " << hardestNucleonTheta_ << "\n"
                  << "\t Hardest nucleon W: " << hardestNucleonW_ << "\n"
                  << "\t PN Weight : "    << weight_ << "\n" 
                  << std::endl;
    }
}
