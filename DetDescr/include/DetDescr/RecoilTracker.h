/*
 * @file Recoil.h
 * @brief Class defining top level DetectorElement for the Recoil Tracker
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_RECOILTRACKER_H_
#define DETDESCR_RECOILTRACKER_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"
#include "DetDescr/GeometryUtil.h"

namespace ldmx {

    /**
     * @class RecoilTracker
     * @brief Top level DetectorElement for the Recoil Tracker
     * @note
     */
    class RecoilTracker : public DetectorElementImpl {

        public:

            RecoilTracker();

            void initialize();

        private:
            DE_INIT(RecoilTracker)
    };

    /**
     * @class RecoilTrackerStation
     * @brief DetectorElement representing a readout station in the Recoil Tracker
     * @note The station number is the support volume copy number.
     * The module number is the copy number modulo 10.
     * The layer number is the copy number divided by 10 using integer division.
     */
    class RecoilTrackerStation : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             * @param support The geometric support.
             * @param layerNum The layer number.
             * @note If the support is non-null, then the layer number is read from the
             * node's copy number if not provided explicitly.
             */
            RecoilTrackerStation(DetectorElementImpl* parent, TGeoNode* support);

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

            /** The station number. */
            int stationNum_{-1};

            /** The layer number. */
            int layerNum_{-1};

            /** The module number. */
            int moduleNum_{-1};
    };
}

#endif /* DETDESCR_RECOILTRACKERDETECTORELEMENT_H_ */
