/*
 * @file Target.h
 * @brief Class defining DetectorElement for the beam target
 * @author JeremyMcCormick, SLAC
 */

#ifndef INCLUDE_DETDESCR_TARGET_H_
#define INCLUDE_DETDESCR_TARGET_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class Target
     * @brief Defines a DetectorElement for the beam target
     */
    class Target : public DetectorElementImpl {

        public:

            Target();

            void initialize();

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

            DE_INIT(Target)
    };
}

#endif /* DETDESCR_TARGETDETECTORELEMENT_H_ */
