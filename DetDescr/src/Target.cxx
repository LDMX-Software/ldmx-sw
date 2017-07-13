#include "DetDescr/Target.h"

// ROOT
#include "TGeoBBox.h"

// LDMX
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    Target::Target() {
        name_ = "Target";
        targetThickness_ = -1.0;
    }

    void Target::initialize() {

        if (!support_) {
            throw std::runtime_error("The Target support is not set.");
        }

        if (!parent_) {
            throw std::runtime_error("The Target parent is not set.");
        }

        // Get target thickness from volume's box shape.
        targetThickness_ = dynamic_cast<TGeoBBox*>(support_->GetVolume()->GetShape())->GetDZ() * 2;

        getDetectorID()->clear();
        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();
    }

    DE_ADD(Target)
}
