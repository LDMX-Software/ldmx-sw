#include "../include/DetDescr/TriggerPad.h"
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    TriggerPad::TriggerPad() {
        name_ = "TriggerPadXXX"; // FIXME
    }

    void TriggerPad::initialize() {
        /*
        support_ = GeometryUtil::findFirstDauNameStartsWith(name_, parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The volume for the trigger pad was not found.");
        }

        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();
        */
    }

    DE_ADD(TriggerPad)
}
