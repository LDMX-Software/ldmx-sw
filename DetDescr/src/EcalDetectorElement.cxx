#include "DetDescr/EcalDetectorElement.h"

#include "DetDescr/DetectorDataService.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/GeometryUtil.h"

#include <iostream>
#include <sstream>

namespace ldmx {

    EcalLayer::EcalLayer(DetectorElementImpl* ecal, TGeoNode* support, int layerNumber) : DetectorElementImpl(ecal, support) {
        layerNumber_ = layerNumber;

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << layerNumber;
        name_ = "EcalLayer" + ss.str();
    }

    EcalDetectorElement::EcalDetectorElement(DetectorElementImpl* parent) : DetectorElementImpl(parent) {

        name_ = "Ecal";

        detID_ = new EcalDetectorID();

        support_ = GeometryUtil::findFirstDauNameStartsWith("Ecal", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Ecal volume was not found.");
        }

        // Add a DE for each ECal active layer.
        auto layerVec = GeometryUtil::findDauNameStartsWith("Si", support_);
        for (auto layerNode : layerVec) {
            new EcalLayer(this, layerNode, layerNode->GetNumber());
        }
    }

    EcalLayer* EcalDetectorElement::getEcalLayer(int num) {
        return static_cast<EcalLayer*>(children_[num - 1]);
    }
}
