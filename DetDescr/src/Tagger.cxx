#include "DetDescr/TaggerDetectorElement.h"

// LDMX
#include "DetDescr/GeometryUtil.h"

// STL
#include <iostream>
#include <sstream>

namespace ldmx {

    TaggerDetectorElement::TaggerDetectorElement(DetectorElementImpl* parent) : DetectorElementImpl(parent) {

        name_ = "Tagger";
        support_ = GeometryUtil::findFirstDauNameStartsWith("Tagger", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Tagger volume was not found.");
        }

       getDetectorID()->clear();
       getDetectorID()->setFieldValue(0, support_->GetNumber());
       id_ = getDetectorID()->pack();

        // Add a DE for each Tagger active layer.
        auto layerVec = GeometryUtil::findDauNameStartsWith("TaggerModuleVolume_physvol", support_);
        for (auto layerNode : layerVec) {
            new TaggerLayer(this, layerNode);
        }
    }

    TaggerLayer::TaggerLayer(DetectorElementImpl* tagger, TGeoNode* support) : DetectorElementImpl(tagger, support) {

        layerNumber_ = support->GetNumber();

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << layerNumber_;
        name_ = "TaggerLayer" + ss.str();

        getDetectorID()->setFieldValue(1, support_->GetNumber());
        id_ = getDetectorID()->pack();
    }
}
