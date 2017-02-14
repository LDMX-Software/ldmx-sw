/*
 * EcalDetectorElement.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef ECALDETECTORELEMENT_H_
#define DETDESCR_ECALDETECTORELEMENT_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

// STL
#include <sstream>
#include <string>

namespace ldmx {

    class EcalLayer : public DetectorElementImpl {

        public:

            EcalLayer(DetectorElementImpl* ecal, TGeoNode* support, int layerNumber);

            int getLayerNumber() {
                return layerNumber_;
            }

        private:

            int layerNumber_{-1};

    };

    class EcalDetectorElement : public DetectorElementImpl {

        public:

            EcalDetectorElement(DetectorElementImpl* parent);

            EcalLayer* getEcalLayer(int num);
    };
}

#endif /* DETDESCR_ECALDETECTORELEMENT_H_ */
