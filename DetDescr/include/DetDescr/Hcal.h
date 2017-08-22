/*
 * @file Hcal.h
 * @brief Class representing the DetectorElement for an HCal station
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_HCAL_H_
#define DETDESCR_HCAL_H_

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class HcalStation
     * @brief DetectorElement for an HCal station
     *
     * @note A station may be a member of any layer in the HCal.
     * The station number is the copy number of the support volume.
     * The layer is the copy number modulo 1000.
     * The section is the copy number divided by 1000 using integer division.
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
             * Get the station number which is the support's volume copy num.
             * @return The station number.
             */
            int getStationNum() {
                return stationNumber_;
            }

            /**
             * Get the logical layer number.
             * @return The layer number.
             */
            int getLayerNum() {
                return layerNum_;
            }

            /**
             * Get the section number (0-4) corresponding to HcalSection enum.
             * @return The section number.
             */
            int getSectionNum() {
                return sectionNum_;
            }

        private:

            /** The HCal station number. */
            int stationNumber_;

            /** The logical layer number of the station. */
            int layerNum_;

            /** The section of the station (0-4). */
            int sectionNum_;
    };

    /**
     * @class Hcal
     * @brief Top level DetectorElement representing the HCal
     */
    class Hcal : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             */
            Hcal();

            void initialize();

        private:
            DE_INIT(Hcal)
    };
}

#endif /* DETDESCR__HCALDETECTORELEMENT_H_ */
