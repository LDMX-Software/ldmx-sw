#include "../include/DetDescr/TriggerPad.h"
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    TriggerPad::TriggerPad(DetectorElementImpl* parent, std::string name) : DetectorElementImpl(parent) {

        name_ = name;

        support_ = GeometryUtil::findFirstDauNameStartsWith(name_, parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The volume for the trigger pad was not found.");
        }

        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();
    }

}
