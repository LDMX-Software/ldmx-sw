/**
 * @file PnWeightResult.cxx
 * @brief Class used to encapsulate the results obtained from
 *        PnWeightProcessor.
 * @author Alex Patterson, UCSB
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
        weight_ = 1.;
    }

    void PnWeightResult::Copy(TObject& object) const {
        PnWeightResult& result = (PnWeightResult&) object;
        result.weight_ = weight_;
    }

    void PnWeightResult::Print(Option_t *option) const {
        std::cout << "[ PnWeightResult ]:\n" << "\t PN Weight : " << weight_ << "\n" << std::endl;
    }

}
