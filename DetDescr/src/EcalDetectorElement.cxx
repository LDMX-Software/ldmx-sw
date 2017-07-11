#include "DetDescr/EcalDetectorElement.h"

// LDMX
#include "DetDescr/DetectorDataService.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/GeometryUtil.h"

// STL
#include <iostream>
#include <sstream>

namespace ldmx {

    EcalLayer::EcalLayer(DetectorElementImpl* parent, TGeoNode* support) : DetectorElementImpl(parent, support) {
        layerNumber_ = support->GetNumber();

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << layerNumber_;
        name_ = "EcalLayer" + ss.str();

        getDetectorID()->setFieldValue(1, support->GetNumber());
        id_ = getDetectorID()->pack();
    }

    EcalDetectorElement::EcalDetectorElement(DetectorElementImpl* parent) : DetectorElementImpl(parent) {

        name_ = "Ecal";

        detectorID_ = new EcalDetectorID();

        support_ = GeometryUtil::findFirstDauNameStartsWith("Ecal", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Ecal volume was not found.");
        }

        detectorID_->clear();
        detectorID_->setFieldValue(0, support_->GetNumber());
        id_ = detectorID_->pack();

        // Add a DE for each ECal active layer.
        auto layerVec = GeometryUtil::findDauNameStartsWith("Si", support_);
        for (auto layerNode : layerVec) {
            new EcalLayer(this, layerNode);
        }
    }

    EcalLayer* EcalDetectorElement::getEcalLayer(int num) {
        return static_cast<EcalLayer*>(children_[num - 1]);
    }

    DE_ADD(EcalDetectorElement)
}
