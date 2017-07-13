#include "../include/DetDescr/TriggerPad.h"
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    TriggerPad::TriggerPad() {
    }

    void TriggerPad::initialize() {
        name_ = support_->GetVolume()->GetName();

        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();
    }

    DE_ADD(TriggerPad)
}
