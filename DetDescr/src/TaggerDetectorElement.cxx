#include "DetDescr/TaggerDetectorElement.h"

#include "DetDescr/GeometryUtil.h"

#include <iostream>

namespace ldmx {

    TaggerDetectorElement::TaggerDetectorElement(DetectorElementImpl* parent) : DetectorElementImpl(parent) {

        name_ = "Tagger";
        support_ = GeometryUtil::findFirstDauNameStartsWith("Tagger", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Tagger volume was not found.");
        }

        // Add a DE for each Tagger active layer.
        auto layerVec = GeometryUtil::findDauNameStartsWith("TaggerModuleVolume_physvol", support_);
        for (auto layerNode : layerVec) {
            new TaggerLayer(this, layerNode);
        }
    }
}
