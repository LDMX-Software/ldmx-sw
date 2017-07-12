#include "DetDescr/TopDetectorElement.h"

// LDMX
#include "DetDescr/DetectorDataService.h"
#include "DetDescr/DefaultDetectorID.h"
#include "DetDescr/EcalDetectorElement.h"
#include "DetDescr/HcalDetectorElement.h"
#include "DetDescr/RecoilTrackerDetectorElement.h"
#include "DetDescr/TaggerDetectorElement.h"
#include "DetDescr/TargetDetectorElement.h"
#include "DetDescr/TriggerPadDetectorElement.h"

#include <iostream>

namespace ldmx {

    TopDetectorElement::TopDetectorElement(TGeoNode* support) : DetectorElementImpl(nullptr, support) {

        // Name of this DE.
        name_ = "Top";

        // Create default detector ID.
        detectorID_ = new DefaultDetectorID();

        // Create top-level detector components.
        /*
        new EcalDetectorElement(this);
        new HcalDetectorElement(this);
        new TaggerDetectorElement(this);
        new RecoilTrackerDetectorElement(this);
        new TargetDetectorElement(this);
        new TriggerPadDetectorElement(this, "TriggerPadDown");
        new TriggerPadDetectorElement(this, "TriggerPadUp");
        */
    }

    DE_ADD(TopDetectorElement)
}
