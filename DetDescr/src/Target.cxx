#include "TGeoBBox.h"

#include "../include/DetDescr/Target.h"
// LDMX
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    Target::Target() {
        name_ = "Target";
        targetThickness_ = -1.0;
    }

    void Target::initialize() {
        /*
        support_ = GeometryUtil::findFirstDauNameStartsWith("Target", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Target volume was not found.");
        }

        // Get target thickness from volume's box shape.
        targetThickness_ = dynamic_cast<TGeoBBox*>(support_->GetVolume()->GetShape())->GetDZ() * 2;

        getDetectorID()->clear();
        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();
        */
    }

    DE_ADD(Target)
}
