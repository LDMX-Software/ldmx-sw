#include "DetDescr/EcalDetectorElement.h"

#include "DetDescr/DetectorDataService.h"
#include "DetDescr/GeometryUtil.h"

#include <iostream>

namespace ldmx {

    EcalDetectorElement::EcalDetectorElement(DetectorElementImpl* parent) : DetectorElementImpl(parent) {

        name_ = "Ecal";

        support_ = GeometryUtil::findFirstDauNameStartsWith("Ecal", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Ecal volume was not found.");
        }

        std::cout << "[ EcalDetectorElement ] : Set support to " << support_->GetName() << std::endl;

        // Add a DE for each ECal active layer.
        auto layerVec = GeometryUtil::findDauNameStartsWith("Si", support_);
        for (auto layerNode : layerVec) {
            std::cout << "[ EcalDetectorElement ] : Found ECal layer node " << layerNode->GetName() << " with copynum " << layerNode->GetNumber() << std::endl;
            new EcalLayer(this, layerNode, layerNode->GetNumber());
        }

        for (auto layer : getChildren()) {
            std::cout << "[ EcalDetectorElement ] : Created ECal layer " << layer->getName() << std::endl;
        }
    }

}
