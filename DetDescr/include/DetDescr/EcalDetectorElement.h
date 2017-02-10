/*
 * EcalDetectorElement.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef ECALDETECTORELEMENT_H_
#define DETDESCR_ECALDETECTORELEMENT_H_

#include "DetDescr/DetectorElementImpl.h"

#include <string>

namespace ldmx {

    class EcalDetectorElement : public DetectorElementImpl {

        public:

            EcalDetectorElement(DetectorElementImpl* parent);
    };

    class EcalLayer : public DetectorElementImpl {

        public:

            EcalLayer(EcalDetectorElement* ecal, TGeoNode* support, int layerNumber) : DetectorElementImpl(ecal) {
                support_ = support;
                layerNumber_ = layerNumber;
                name_ = "EcalLayer" + std::to_string(layerNumber);
            }

            int getLayerNumber() {
                return layerNumber_;
            }

        private:

            int layerNumber_{-1};

    };
}




#endif /* DETDESCR_ECALDETECTORELEMENT_H_ */
