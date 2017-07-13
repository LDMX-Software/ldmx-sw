/*
 * RecoilTrackerDetectorElement.h
 * @brief Class defining top level DetectorElement for the Recoil Tracker
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_RECOILTRACKERDETECTORELEMENT_H_
#define DETDESCR_RECOILTRACKERDETECTORELEMENT_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    /**
     * @class RecoilTrackerDetectorElement
     * @brief Top level DetectorElement for the Recoil Tracker
     */
    class RecoilTracker : DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             */
            RecoilTracker(DetectorElementImpl* parent);

    };

    /**
     * @class RecoilTrackerLayer
     * @brief DetectorElement representing a layer in the Recoil Tracker
     */
    class RecoilTrackerLayer : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             * @param support The geometric support.
             * @param layerNum The layer number.
             * @note If the support is non-null, then the layer number is read from the
             * node's copy number if not provided explicitly.
             */
            RecoilTrackerLayer(DetectorElementImpl* parent, TGeoNode* support, int layerNum = -1);

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
     * @class RecoilTrackerSensor
     * @brief DetectorElement representing a sensor in the Recoil Tracker
     */
    class RecoilTrackerSensor : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param layer The parent RecoilTrackerLayer DetectorElement.
             * @param support The geometric support.
             */
            RecoilTrackerSensor(RecoilTrackerLayer* layer, TGeoNode* support);

            /**
             * Get the sensor number.
             * @return The sensor number.
             * @note The sensor numbers are numbered from 1, not 0.
             */
            int getSensorNumber() {
                return sensorNumber_;
            }

        private:

            /** The sensor number. */
            int sensorNumber_{-1};
    };

}

#endif /* DETDESCR_RECOILTRACKERDETECTORELEMENT_H_ */
