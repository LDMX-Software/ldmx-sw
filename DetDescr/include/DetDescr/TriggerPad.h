/*
 * TriggerPadDetectorElement.h
 * @brief Class defining a DetectorElement for the trigger pads
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TRIGGERPADDETECTORELEMENT_H_
#define DETDESCR_TRIGGERPADDETECTORELEMENT_H_

#include <string>

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class TriggerPadDetectorElement
     * @brief Defines a DetectorElement for a trigger pad
     */
    class TriggerPad : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             * @param name The name of the trigger pad in the geometry.
             */
            TriggerPad(DetectorElementImpl* parent, std::string name);
    };
}

#endif /* DETDESCR_TRIGGERPADDETECTORELEMENT_H_ */
