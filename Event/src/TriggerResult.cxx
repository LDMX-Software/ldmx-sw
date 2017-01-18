#include "Event/TriggerResult.h"

ClassImp(event::TriggerResult)

namespace event {

TriggerResult::TriggerResult()
    : TObject() {
}

TriggerResult::~TriggerResult() {
    Clear();
}

void TriggerResult::Print(Option_t *option) const {
    std::cout << "TriggerResult { " << "name: " << name_ << 
    ", " << "pass: " << pass_ << " }" << std::endl;

    for (int i = 0; i < variables_.GetSize(); ++i) {
        std::cout << "Element " << i << " : " << variables_[i] << std::endl; 
    }
}

void TriggerResult::Clear(Option_t*) {
    TObject::Clear();

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

void TriggerResult::setVar(int element, double value) {

    if (element >= 0 && element < variables_.GetSize()) {
        variables_[element] = value;
    }
}
    
}
