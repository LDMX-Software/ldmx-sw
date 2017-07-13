#include "DetDescr/DetectorDataService.h"
#include "DetDescr/DefaultDetectorID.h"
#include <iostream>
#include "../include/DetDescr/Ecal.h"
#include "../include/DetDescr/Hcal.h"
#include "../include/DetDescr/RecoilTracker.h"
#include "../include/DetDescr/Tagger.h"
#include "../include/DetDescr/Target.h"
#include "../include/DetDescr/Top.h"
#include "../include/DetDescr/TriggerPad.h"

namespace ldmx {

    Top::Top() {
        // Name of this DE.
        name_ = "Top";

        // Create default detector ID.
        detectorID_ = new DefaultDetectorID();
    }

    Top::~Top() {
    }

    DE_ADD(Top)
}
