#include "DetDescr/HcalDetectorElement.h"

#include "DetDescr/GeometryUtil.h"

#include <iostream>
#include <sstream>

namespace ldmx {

    HcalStation::HcalStation(DetectorElementImpl* parent, TGeoNode* support) : DetectorElementImpl(parent, support) {
        stationNumber_ = support->GetNumber();

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << stationNumber_;
        name_ = "HcalStation" + ss.str();

        // TODO: Set detector ID value.
    }

    HcalDetectorElement::HcalDetectorElement(DetectorElementImpl* parent) : DetectorElementImpl(parent) {

        name_ = "Hcal";

        //detID_ = new HcalDetectorID();

        support_ = GeometryUtil::findFirstDauNameStartsWith("Hcal", parent->getSupport());
        if (!support_) {
            throw std::runtime_error("The Hcal volume was not found.");
        }

        getDetectorID()->clear();
        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();

        int nTopDau = support_->GetNdaughters();
        for (int iDau = 0; iDau < nTopDau; iDau++) {
            auto dau = support_->GetDaughter(iDau);
            if (dau->GetNumber()) {
                new HcalStation(this, dau);
            }
        }
    }

    HcalStation* HcalDetectorElement::getHcalStation(int num) {
        return static_cast<HcalStation*>(children_[num - 1]);
    }

}
