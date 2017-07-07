/*
 * HcalDetectorElement.h
 * @brief Class representing the DetectorElement for an HCal station
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_HCALDETECTORELEMENT_H_
#define DETDESCR_HCALDETECTORELEMENT_H_

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class HcalStation
     * @brief Represents the DetectorElement for an HCal station
     *
     * @note A station may be a member of any layer in the HCal.
     * Stations are numbered sequentially according to the copy numbers
     * read from the geometry supports.  A DetectorID should be used to
     * decode its number into a layer number.
     */
    class HcalStation : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             * @param support The geometric support.
             * @note The station number is read from the support's copy number.
             */
            HcalStation(DetectorElementImpl* parent, TGeoNode* support);

            /**
             * Get the station number.
             * @return The station number.
             */
            int getStationNumber() {
                return stationNumber_;
            }

        private:

            /** The HCal station number. */
            int stationNumber_;
    };

    /**
     * @class HcalDetectorElement
     * @brief Top level DetectorElement representing the HCal
     */
    class HcalDetectorElement : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             */
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
