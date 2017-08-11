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
     * @class EcalLayer
     * @brief DetectorElement definition for each ECal Si sensor station
     */
    class EcalStation : public DetectorElementImpl {

        public:

            EcalStation(DetectorElementImpl* parent, TGeoNode* support);

            /**
             * Get the layer number.
             * @return The layer number.
             */
            int getLayerNumber() {
                return layerNumber_;
            }

            /**
             * Get the module number.
             * @return The module number.
             */
            int getModuleNumber() {
                return moduleNumber_;
            }

        private:

            /** The layer number (0-33 in v3 geometry). */
            int layerNumber_{-1};

            /** The module number (0-7). */
            int moduleNumber_{-1};

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

            /**
             * Get the EcalLayer object by its layer number.
             * @param layerNumber The layer number.
             * @note The layers are numbered from 1, not 0.
             */
            EcalStation* getEcalStation(int stationNumber);

            void initialize();

        private:
            DE_INIT(Ecal)
    };
}

#endif /* DETDESCR_ECALDETECTORELEMENT_H_ */
