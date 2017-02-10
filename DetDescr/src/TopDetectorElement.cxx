#include "DetDescr/TopDetectorElement.h"

#include "DetDescr/DetectorDataService.h"
#include "DetDescr/DefaultDetectorID.h"
#include "DetDescr/EcalDetectorElement.h"

#include <iostream>

namespace ldmx {

    TopDetectorElement::TopDetectorElement(TGeoNode* support) : DetectorElementImpl(nullptr, support) {

        name_ = "Top";

        // detector ID
        detID_ = new DefaultDetectorID();

        // Add DE for detector components.
        new EcalDetectorElement(this);

        // TODO: add child DE for...
        // Hcal
        // RecoilTracker
        // Tagger
        // Target
        // TriggerPad
    }
}
