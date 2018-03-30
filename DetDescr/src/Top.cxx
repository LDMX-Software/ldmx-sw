#include "DetDescr/Top.h"

#include "DetDescr/DefaultDetectorID.h"

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
