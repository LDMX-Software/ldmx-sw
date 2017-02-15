#include "DetDescr/TargetDetectorElement.h"

// ROOT
#include "TGeoBBox.h"

// LDMX
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    TargetDetectorElement::TargetDetectorElement(DetectorElementImpl* parent) : DetectorElementImpl(parent) {

        name_ = "Target";

        support_ = GeometryUtil::findFirstDauNameStartsWith("Target", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Target volume was not found.");
        }

        // Get target thickness from volume's box shape.
        targetThickness_ = dynamic_cast<TGeoBBox*>(support_->GetVolume()->GetShape())->GetDZ() * 2;
    }

}
