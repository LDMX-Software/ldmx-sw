#include "DetDescr/Ecal.h"

// LDMX
#include "DetDescr/DetectorElement.h"
#include "DetDescr/EcalDetectorID.h"

// ROOT
#include <Rtypes.h>
#include <TGeoNode.h>
#include <TNamed.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ldmx {

    EcalStation::EcalStation(DetectorElementImpl* parent, TGeoNode* support) : DetectorElementImpl(parent, support) {

        int copynum = support_->GetNumber();

        // Follows v3 convention where layer number is copynum divided by 7. --JMc
        layerNumber_ = copynum / 7;

        // Follows v3 convention where module number is copynum modulo 7. --JMc
        module_ = copynum % 7;

        // Ecal stations are named using the copynum.
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << copynum;
        name_ = "EcalStation" + ss.str();

        // Create the bit-packed ID for the detector component.
        auto detID = getDetectorID();
        detID->setFieldValue(1, layerNumber_);
        detID->setFieldValue(2, module_);
        id_ = getDetectorID()->pack();
    }

    EcalStation* Ecal::getEcalStation(int num) {
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
            // Sensor names must start with "Si_" in the GDML!
            static std::string prefix = "Si_";
            if (!std::string(dau->GetName()).compare(0, prefix.size(), prefix)) {
                new EcalStation(this, dau);
            }
        }
    }

    DE_ADD(Ecal)
}
