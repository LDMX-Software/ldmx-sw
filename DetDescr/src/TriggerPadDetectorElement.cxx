#include "DetDescr/TriggerPadDetectorElement.h"

#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    TriggerPadDetectorElement::TriggerPadDetectorElement(DetectorElementImpl* parent, std::string name) : DetectorElementImpl(parent) {

        name_ = name;

        support_ = GeometryUtil::findFirstDauNameStartsWith(name_, parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The volume for the trigger pad was not found.");
        }
    }

}
