#include "DetDescr/TopDetectorElement.h"

// LDMX
#include "DetDescr/DetectorDataService.h"
#include "DetDescr/DefaultDetectorID.h"
#include "DetDescr/EcalDetectorElement.h"
#include "DetDescr/HcalDetectorElement.h"
#include "DetDescr/RecoilTrackerDetectorElement.h"
#include "DetDescr/TaggerDetectorElement.h"
#include "DetDescr/TargetDetectorElement.h"

#include <iostream>

namespace ldmx {

    TopDetectorElement::TopDetectorElement(TGeoNode* support) : DetectorElementImpl(nullptr, support) {

        // Name of this DE.
        name_ = "Top";

        // Create default detector ID.
        detID_ = new DefaultDetectorID();

        // Create top-level detector components.
        new EcalDetectorElement(this);
        new HcalDetectorElement(this);
        new TaggerDetectorElement(this);
        new RecoilTrackerDetectorElement(this);
        new TargetDetectorElement(this);

        // TODO: Add DE for...
        // TriggerPad
    }
}
