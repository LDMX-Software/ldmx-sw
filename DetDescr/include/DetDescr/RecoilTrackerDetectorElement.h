/*
 * RecoilTrackerDetectorElement.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_RECOILTRACKERDETECTORELEMENT_H_
#define DETDESCR_RECOILTRACKERDETECTORELEMENT_H_

#include "DetDescr/DetectorElementImpl.h"
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    class RecoilTrackerDetectorElement : DetectorElementImpl {

        public:

            RecoilTrackerDetectorElement(DetectorElementImpl* parent);

    };

    class RecoilTrackerLayer : public DetectorElementImpl {

        public:

            RecoilTrackerLayer(DetectorElementImpl* parent, TGeoNode* support, int layerNum = -1);

            int getLayerNumber() {
                return layerNumber_;
            }

        private:

            int layerNumber_{-1};
    };

    class RecoilTrackerSensor : public DetectorElementImpl {

        public:

            RecoilTrackerSensor(RecoilTrackerLayer* layer, TGeoNode* support);

            int getSensorNumber() {
                return sensorNumber_;
            }

        private:

            int sensorNumber_{-1};
    };

}

#endif /* DETDESCR_RECOILTRACKERDETECTORELEMENT_H_ */
