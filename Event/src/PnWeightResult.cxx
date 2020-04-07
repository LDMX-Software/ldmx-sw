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

    PnWeightResult::PnWeightResult() { }

    PnWeightResult::~PnWeightResult() {
        Clear();
    }

    void PnWeightResult::Clear() {

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

    void PnWeightResult::Print(std::ostream& o) const {
        o << "PnWeightResult {"
                  << "Hardest nucleon KE: " << hardestNucleonKe_ 
                  << ", Hardest nucleon theta: " << hardestNucleonTheta_
                  << ", Hardest nucleon W: " << hardestNucleonW_
                  << ", PN Weight : "    << weight_ << "}";
    }
}
