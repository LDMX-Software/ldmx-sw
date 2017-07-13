#include "../include/DetDescr/TriggerPad.h"
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    TriggerPad::TriggerPad() {
    }

    void TriggerPad::initialize() {

        if (!support_) {
            throw std::runtime_error("The TriggerPad support is not set.");
        }

        if (!parent_) {
            throw std::runtime_error("The TriggerPad parent is not set.");
        }

        name_ = support_->GetVolume()->GetName();

        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();
    }

    DE_ADD(TriggerPad)
}
