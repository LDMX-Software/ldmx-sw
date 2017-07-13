#include "DetDescr/Tagger.h"

// LDMX
#include "DetDescr/GeometryUtil.h"

// STL
#include <iostream>
#include <sstream>


namespace ldmx {

    Tagger::Tagger() {
        name_ = "Tagger";
    }

    void Tagger::initialize() {

        if (!support_) {
            throw std::runtime_error("The Tagger support is not set.");
        }

        if (!parent_) {
            throw std::runtime_error("The Tagger parent is not set.");
        }

        getDetectorID()->clear();
        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();

        // Add a DE for each Tagger active layer.
        auto layerVec = GeometryUtil::findDauNameStartsWith("LDMXTaggerModuleVolume_physvol", support_);
        for (auto layerNode : layerVec) {
            auto layer = new TaggerStation(this, layerNode);
        }
    }

    TaggerStation::TaggerStation(DetectorElementImpl* tagger, TGeoNode* support) : DetectorElementImpl(tagger, support) {

        layerNumber_ = support->GetNumber();

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << layerNumber_;
        name_ = "TaggerLayer" + ss.str();

        getDetectorID()->setFieldValue(1, support_->GetNumber());
        id_ = getDetectorID()->pack();
    }

    DE_ADD(Tagger)
}
