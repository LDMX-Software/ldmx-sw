/*
 * TriggerPad.h
 * @brief Class defining a DetectorElement for the trigger pads
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TRIGGERPAD_H_
#define DETDESCR_TRIGGERPAD_H_

#include <string>

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class TriggerPad
     * @brief Defines a DetectorElement for a trigger pad
     */
    class TriggerPad : public DetectorElementImpl {

        public:

            TriggerPad();

            void initialize();

        private:
            DE_INIT(TriggerPad)
    };
}

#endif /* DETDESCR_TRIGGERPADDETECTORELEMENT_H_ */
