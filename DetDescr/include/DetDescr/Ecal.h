/*
 * @file Ecal.h
 * @brief Class providing a DetectorElement for the ECal
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_ECAL_H_
#define DETDESCR_ECAL_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class EcalStation
     * @brief DetectorElement for each ECal sensor station
     */
    class EcalStation : public DetectorElementImpl {

        public:

            EcalStation(DetectorElementImpl* parent, TGeoNode* support);

            /**
             * Get the layer number.
             * @return The layer number.
             */
            int getLayerNum() {
                return layerNum_;
            }

            /**
             * Get the module number.
             * @return The module number.
             */
            int getModuleNum() {
                return moduleNum_;
            }

        private:

            /** The layer number (0-33 in v3 geometry). */
            int layerNum_{-1};

            /** The module number (0-7). */
            int moduleNum_{-1};

    };

    /**
     * @class Ecal
     * @brief The top-level DetectorElement for the ECal
     * @note Compatible with the v3 GDML detector definition.
     */
    class Ecal : public DetectorElementImpl {

        public:

            Ecal();

            ~Ecal();

            void initialize();

        private:
            DE_INIT(Ecal)
    };
}

#endif /* DETDESCR_ECALDETECTORELEMENT_H_ */
