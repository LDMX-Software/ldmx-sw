#include "Event/TriggerResult.h"

ClassImp(ldmx::TriggerResult)

namespace ldmx {

    TriggerResult::TriggerResult() {
    }

    TriggerResult::~TriggerResult() {
        Clear();
    }

    void TriggerResult::Print(std::ostream& o) const {
        o << "TriggerResult { " << "name: " << name_ << ", " << "pass: " << pass_ << " }";
    }

    void TriggerResult::Clear() {

        name_ = "";
        pass_ = false;

        for (int i = 0; i < variables_.GetSize(); ++i) {
            variables_[i] = 0;
        }
    }

    void TriggerResult::set(const TString& name, bool pass, int nvar) {

        name_ = name;
        pass_ = pass;

        if (nvar > variables_.GetSize()) {
            variables_.Set(nvar);
        }
    }

    void TriggerResult::setAlgoVar(int element, double value) {
        if (element >= 0 && element < variables_.GetSize()) {
            variables_[element] = value;
        }
    }

}
