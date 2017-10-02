#include "DetDescr/Hcal.h"

// LDMX
#include "DetDescr/GeometryUtil.h"
#include "DetDescr/HcalID.h"

// C++
#include <iostream>
#include <sstream>

namespace ldmx {

    HcalStation::HcalStation(DetectorElementImpl* parent, TGeoNode* support) : DetectorElementImpl(parent, support) {

        int copyNum = support->GetNumber();
        int layerNum_ = copyNum % 1000;
        int sectionNum_ = copyNum / 1000;

        auto detID = parent->getDetectorID();
        detID->setFieldValue(1, layerNum_);
        detID->setFieldValue(2, sectionNum_);
        this->id_ = detID->pack();

        this->stationNumber_ = copyNum;

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << stationNumber_;
        name_ = "HcalStation" + ss.str();
    }

    Hcal::Hcal() {
        name_ = "Hcal";
        detectorID_ = new HcalID();
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

    DE_ADD(Hcal)
}
