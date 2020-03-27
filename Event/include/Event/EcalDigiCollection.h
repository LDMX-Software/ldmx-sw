/**
 * @file EcalDigiCollection.h
 * @brief Class that represents a digitized hit in a calorimeter cell within the detector
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENT_ECALDIGICOLLECTION_H_
#define EVENT_ECALDIGICOLLECTION_H_

// LDMX
#include "Event/DigiCollection.h"

namespace ldmx {

    /**
     * @struct EcalDigiSample
     * @brief One sample of an Ecal digi channel
     *
     * Usually several samples are used for each channel to re-construct the hit.
     */
    struct EcalDigiSample {
        /** Raw integer ID of channel this sample is for */
        int rawID_{-1};

        /** ADC counts in this channel at this time */
        int adc_t_{0};

        /** ADC counts in this channel at the previous time */
        int adc_tm1_{0};

        /** Time counts over threshhold in this channel */
        int tot_{0};

        /** Time counts when signal arrived in this channel */
        int toa_{0};
    };

    /**
     * @class EcalDigiCollection
     * @brief Represents a collection of the ECal digi hits
     *
     * @note This class represents the digitized hit information
     * from the Ecal
     */
    class EcalDigiCollection : public DigiCollection {

        public:

            /**
             * Class constructor.
             */
            EcalDigiCollection() {
            }

            /**
             * Class destructor.
             */
            virtual ~EcalDigiCollection() {
            }

            /**
             * Get samples for the input digi index
             */
            std::vector< EcalDigiSample > getDigi( unsigned int digiIndex ) const;

            /**
             * Translate and add samples to collection
             */
            void addDigi( std::vector< EcalDigiSample > newSamples );

        private:

            /** Mask for lowest order bit in an int */
            static const int ONE_BIT_MASK = 1;

            /** Mask for lowest order ten bits in an int */
            static const int TEN_BIT_MASK = (1 << 10) - 1;

            /** Bit position of first flag */
            static const int FIRSTFLAG_POS = 31;

            /** Bit position of second flag */
            static const int SECONFLAG_POS = 30;

            /** Bit position of first measurement */
            static const int FIRSTMEAS_POS = 20;

            /** Bit position of second measurement */
            static const int SECONMEAS_POS = 10;

            /**
             * Get and Translate sample to the four measurements that could be encoded
             */
            EcalDigiSample getSample( unsigned int digiIndex , unsigned int sampleIndex ) const;

            /**
             * The ROOT class definition.
             */
            ClassDef(EcalDigiCollection, 1);
    };

}

#endif /* EVENT_ECALDIGI_H_ */
