/*
 * @file Tagger.h
 * @brief Class defining top level DetectorElement for the Tagger
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TAGGER_H_
#define DETDESCR_TAGGER_H_

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class Tagger
     * @brief Defines top level DetectorElement for the Tagger
     */
    class Tagger : public DetectorElementImpl {

        public:

            Tagger();

            void initialize();

        private:
            DE_INIT(Tagger)
    };


    /**
     * @class TaggerLayer
     * @brief DetectorElement representing a station in the Tagger Tracker
     */
    class TaggerStation : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             * @param support The geomtric support.
             * @note The layer number is read from the support's copy number.
             */
            TaggerStation(DetectorElementImpl* parent, TGeoNode* support);

            /**
             * Get the layer number.
             * @return The layer number.
             * @note The layer numbers are numbered from 1, not 0.
             */
            int getLayerNum() {
                return layerNum_;
            }

        private:

            /** The layer number. */
            int layerNum_{-1};
    };
}

#endif /* DETDESCR_TAGGERDETECTORELEMENT_H_ */
