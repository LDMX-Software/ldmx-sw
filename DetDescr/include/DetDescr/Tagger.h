/*
 * TaggerDetectorElement.h
 * @brief Class defining top level DetectorElement for the Tagger
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TAGGERDETECTORELEMENT_H_
#define DETDESCR_TAGGERDETECTORELEMENT_H_

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class TaggerDetectorElement
     * @brief Defines top level DetectorElement for the Tagger
     */
    class Tagger : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             */
            Tagger();

            void initialize();

        private:
            DE_INIT(Tagger)
    };


    /**
     * @class TaggerLayer
     * @brief DetectorElement representing a layer in the Tagger
     */
    class TaggerLayer : public DetectorElementImpl {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement.
             * @param support The geomtric support.
             * @note The layer number is read from the support's copy number.
             */
            TaggerLayer(DetectorElementImpl* parent, TGeoNode* support);

            /**
             * Get the layer number.
             * @return The layer number.
             * @note The layer numbers are numbered from 1, not 0.
             */
            int getLayerNumber() {
                return layerNumber_;
            }

        private:

            /** The layer number. */
            int layerNumber_{-1};
    };
}

#endif /* DETDESCR_TAGGERDETECTORELEMENT_H_ */
