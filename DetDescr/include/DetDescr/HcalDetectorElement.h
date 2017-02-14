/*
 * HcalDetectorElement.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_HCALDETECTORELEMENT_H_
#define DETDESCR_HCALDETECTORELEMENT_H_

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class HcalStation
     * @brief Represents the DetectorElement for an Hcal station
     *
     * @note A station may be a member of any layer in the Hcal.
     * A DetectorID should be used to decode into a layer number.
     */
    class HcalStation : public DetectorElementImpl {

        public:

            HcalStation(DetectorElementImpl* parent, TGeoNode* support);

            int getStationNumber() {
                return stationNumber_;
            }

        private:

            int stationNumber_;
    };

    class HcalDetectorElement : public DetectorElementImpl {

        public:

            HcalDetectorElement(DetectorElementImpl* parent);

            /**
             * Get the DetectorElement for an Hcal station.
             * These are numbered from 1, not 0.
             * @param num The number of the HcalStation.
             * @return The HcalStation with the matching number.
             */
            HcalStation* getHcalStation(int num);
    };
}

#endif /* DETDESCR__HCALDETECTORELEMENT_H_ */
