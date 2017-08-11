#include "DetDescr/Hcal.h"

// LDMX
#include "DetDescr/GeometryUtil.h"

// C++
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

    Hcal::Hcal() {

        name_ = "Hcal";
        //detID_ = new HcalDetectorID();
    }

    void Hcal::initialize() {

        if (!support_) {
            throw std::runtime_error("The Hcal support is not set.");
        }

        if (!parent_) {
            throw std::runtime_error("The Hcal parent is not set.");
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

    HcalStation* Hcal::getHcalStation(int num) {
        return static_cast<HcalStation*>(children_[num - 1]);
    }

    DE_ADD(Hcal)
}
