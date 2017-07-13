/*
 * Ecal.h
 * @brief Class providing a DetectorElement for the ECal
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_ECAL_H_
#define DETDESCR_ECAL_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class EcalLayer
     * @brief DetectorElement for ECal layers
     */
    class EcalStation : public DetectorElementImpl {

        public:

            EcalStation();

            void initialize();

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
     * @class Ecal
     * @brief The top-level DetectorElement for the ECal.
     */
    class Ecal : public DetectorElementImpl {

        public:

            Ecal();

            ~Ecal();

            /**
             * Get the EcalLayer object by its layer number.
             * @param layerNumber The layer number.
             * @note The layers are numbered from 1, not 0.
             */
            EcalStation* getEcalLayer(int layerNumber);

            void initialize();

        private:
            DE_INIT(Ecal)
    };
}

#endif /* DETDESCR_ECALDETECTORELEMENT_H_ */
