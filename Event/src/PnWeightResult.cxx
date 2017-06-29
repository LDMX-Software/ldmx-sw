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

    void PnWeightResult::setResult(double ke, double theta, double w, double fitW, double weight){
        fitW_ = fitW;
        ke_ = ke;  
        w_ = w;
        theta_ = theta; 
        weight_ = weight;
    }

    void PnWeightResult::Clear(Option_t *option) {
        TObject::Clear();

        fitW_ = 0.0;
        ke_ = 0; 
        w_ = 0.0;
        theta_ = 0.0;
        weight_ = 0.0;
    }

    void PnWeightResult::Copy(TObject& object) const { 
        PnWeightResult& result = (PnWeightResult&) object; 

        result.fitW_ = fitW_;
        result.ke_ = ke_; 
        result.theta_ = theta_;  
        result.w_ = w_; 
        result.weight_ = weight_; 
    }

    void PnWeightResult::Print(Option_t *option) const {
        std::cout << "[ PnWeightResult ]:\n" 
                  << "\t W fit : "      << fitW_ << "\n"
                  << "\t PN daughter kinetic energy: " << ke_ << "\n"
                  << "\t W measured : " << w_ << "\n"
                  << "\t PN daughter polar angle: " << theta_ << "\n"
                  << "\t PN Weight : "    << weight_ << "\n" 
                  << std::endl;
    }
}
