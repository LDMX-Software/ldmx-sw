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

    void PnWeightResult::setResult(double weight, double measuredWp, double wpFit) { 
        
        weight_ = weight;
        measuredWp_ = measuredWp;
        fitWp_ = wpFit; 
    }

    void PnWeightResult::Clear(Option_t *option) {
        TObject::Clear();
        weight_ = 0.0;
        measuredWp_ = 0.0;
        fitWp_ = 0.0; 
    }

    void PnWeightResult::Copy(TObject& object) const { 
        PnWeightResult& result = (PnWeightResult&) object; 
        result.weight_ = weight_; 
        result.measuredWp_ = measuredWp_; 
        result.fitWp_ = fitWp_; 
    }

    void PnWeightResult::Print(Option_t *option) const {
        std::cout << "[ PnWeightResult ]:\n" 
                  << "\t PN Weight : "    << weight_ << "\n" 
                  << "\t W_p measured : " << measuredWp_ << "\n" 
                  << "\t W_p fit : "      << fitWp_ << "\n" 
                  << std::endl;
    }
}
