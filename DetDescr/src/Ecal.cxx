#include <Rtypes.h>
#include <TGeoNode.h>
#include <TNamed.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../include/DetDescr/DetectorElement.h"
#include "../include/DetDescr/Ecal.h"
#include "../include/DetDescr/EcalDetectorID.h"

namespace ldmx {

    EcalStation::EcalStation() {
    }

    void EcalStation::initialize() {
        layerNumber_ = support_->GetNumber();

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << layerNumber_;
        name_ = "EcalStation" + ss.str();

        getDetectorID()->setFieldValue(1, support_->GetNumber());
        id_ = getDetectorID()->pack();
    }

    EcalStation* Ecal::getEcalLayer(int num) {
        return static_cast<EcalStation*>(children_[num - 1]);
    }

    Ecal::Ecal() {
        name_ = "Ecal";
        detectorID_ = new EcalDetectorID();
    }

    Ecal::~Ecal() {
    }

    void Ecal::initialize() {

        if (!support_) {
            throw std::runtime_error("The Ecal support is not set.");
        }

        if (!parent_) {
            throw std::runtime_error("The Ecal parent is not set.");
        }

        detectorID_->clear();
        detectorID_->setFieldValue(0, support_->GetNumber());
        id_ = detectorID_->pack();

        int nDau = this->support_->GetNdaughters();
        TGeoNode* dau = nullptr;
        for (int iDau = 0; iDau < nDau; iDau++) {
            auto dau = support_->GetDaughter(iDau);
            static std::string prefix = "Si";
            if (!std::string(dau->GetName()).compare(0, prefix.size(), prefix)) {
                auto ecalStation = new EcalStation();
                ecalStation->setSupport(dau);
                ecalStation->setParent(this);
                ecalStation->initialize();
            }
        }
    }

    DE_ADD(Ecal)
}
