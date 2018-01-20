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

        theta_.clear(); 
        w_.clear(); 

        hardestNucleonKe_ = -9999;
        hardestNucleonTheta_ = -9999;
        hardestNucleonW_ = -9999; 
        highestWNucleonKe_ = -9999;
        highestWNucleonTheta_ = -9999;
        highestWNucleonW_ = -9999; 
        weight_ = 1.0; 
    }

    void PnWeightResult::Copy(TObject& object) const { 
        PnWeightResult& result = (PnWeightResult&) object; 
            
        result.theta_ = theta_;
        result.w_ = w_; 

        result.hardestNucleonKe_ = hardestNucleonKe_;
        result.hardestNucleonTheta_ = hardestNucleonTheta_;
        result.hardestNucleonW_ = hardestNucleonW_; 
        result.highestWNucleonKe_ = highestWNucleonKe_;
        result.highestWNucleonTheta_ = highestWNucleonTheta_;
        result.highestWNucleonW_ = highestWNucleonW_; 
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
