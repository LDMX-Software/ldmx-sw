/*
 * TargetDetectorElement.h
 * @brief Class defining DetectorElement for the beam target
 * @author JeremyMcCormick, SLAC
 */

#ifndef INCLUDE_DETDESCR_TARGETDETECTORELEMENT_H_
#define INCLUDE_DETDESCR_TARGETDETECTORELEMENT_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class TargetDetectorElement
     * @brief Defines a DetectorElement for the beam target
     */
    class TargetDetectorElement : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             */
            TargetDetectorElement(DetectorElementImpl* parent);

            /**
             * Get the thickness of the target [mm].
             * @return The thickness of the target.
             */
            double getTargetThickness() {
                return targetThickness_;
            }

        private:

            /** The thickness of the target from the Z dimension of the box. */
            double targetThickness_;
    };
}

#endif /* DETDESCR_TARGETDETECTORELEMENT_H_ */
