/*
 * EcalDetectorElement.h
 * @brief Class providing a DetectorElement for the ECal
 * @author JeremyMcCormick, SLAC
 */

#ifndef ECALDETECTORELEMENT_H_
#define DETDESCR_ECALDETECTORELEMENT_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class EcalLayer
     * @brief DetectorElement for ECal layers
     */
    class EcalLayer : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             * @param support The geometric support.
             * @note The layer number is read from the node's copy number.
             */
            EcalLayer(DetectorElementImpl* parent, TGeoNode* support);

            /**
             * Get the layer number.
             * @return The layer number.
             */
            int getLayerNumber() {
                return layerNumber_;
            }

        private:

            /** The layer number. */
            int layerNumber_{-1};

    };

    /**
     * @class EcalDetectorElement
     * @brief The top-level DetectorElement for the ECal.
     */
    class EcalDetectorElement : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             */
            EcalDetectorElement(DetectorElementImpl* parent);

            /**
             * Get the EcalLayer object by its layer number.
             * @param layerNumber The layer number.
             * @note The layers are numbered from 1, not 0.
             */
            EcalLayer* getEcalLayer(int layerNumber);
    };
}

#endif /* DETDESCR_ECALDETECTORELEMENT_H_ */
