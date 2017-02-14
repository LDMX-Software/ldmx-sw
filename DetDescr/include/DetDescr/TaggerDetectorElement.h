/*
 * TaggerDetectorElement.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TAGGERDETECTORELEMENT_H_
#define DETDESCR_TAGGERDETECTORELEMENT_H_

#include "DetDescr/DetectorElementImpl.h"

#include <sstream>

namespace ldmx {

    class TaggerDetectorElement : public DetectorElementImpl {

        public:

            TaggerDetectorElement(DetectorElementImpl* parent);
    };

    class TaggerLayer : public DetectorElementImpl {

        public:

            TaggerLayer(DetectorElementImpl* tagger, TGeoNode* support) : DetectorElementImpl(tagger, support) {

                layerNumber_ = support->GetNumber();

                std::stringstream ss;
                ss << std::setfill('0') << std::setw(2) << layerNumber_;
                name_ = "TaggerLayer" + ss.str();
            }

            int getLayerNumber() {
                return layerNumber_;
            }

        private:

            int layerNumber_{-1};
    };
}


#endif /* DETDESCR_TAGGERDETECTORELEMENT_H_ */
