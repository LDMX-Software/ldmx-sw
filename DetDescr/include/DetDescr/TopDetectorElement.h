/*
 * TopDetectorElement.h
 * @brief Top DetectorElement in the hierarchy providing access to the major subsystems
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TOPDETECTORELEMENT_H_
#define DETDESCR_TOPDETECTORELEMENT_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class TopDetectorElement
     * @brief Sets up the main detector subsystems such as ECal, Recoil Tracker, etc.
     */
    class TopDetectorElement : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param support The geometric support (points to world volume).
             */
            TopDetectorElement(TGeoNode* support);
    };
}


#endif /* DETDESCR_TOPDETECTORELEMENT_H_ */
