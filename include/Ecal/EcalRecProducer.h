/**
 * @file EcalRecProducer.h
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef ECAL_ECALRECPRODUCER_H_
#define ECAL_ECALRECPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <time.h>
#include <memory> //for smart pointers

//----------//
//   ROOT   //
//----------//

//----------//
//   LDMX   //
//----------//
#include "Event/EventDef.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    /**
     * @class EcalRecProducer
     * @brief Performs basic ECal reconstruction
     *
     * Reconstruction is done from the EcalDigi samples.
     * Some hard-coded parameters are used for position and energy calculation.
     */
    class EcalRecProducer : public Producer {

        public:

            /**
             * Constructor
             */
            EcalRecProducer(const std::string& name, Process& process);

            /**
             * Destructor
             * Deletes hanging pointers if they exist
             */
            virtual ~EcalRecProducer();

            /**
             * Grabs configure parameters from the python config file.
             *
             * Parameter        Default
             * digiCollName     EcalDigis
             * digiPassName     "" <-- blank means take any pass if only one collection exists
             * layerWeights     DEFAULT_LAYER_WEIGHTS
             * secondOrderEnergyCorrection DEFAULT_SECOND_ORDER_ENERGY_CORRECTION
             */
            virtual void configure(Parameters&);

            /**
             * Produce EcalHits and put them into the event bus using the
             * EcalDigis as input.
             */
            virtual void produce(Event& event);

        private:

            /**
             * Convert TOT from digis to a reconstructed energy deposited in the Silicon.
             *
             * We construct this conversion by fitting the plot of TOT vs Sim Energy Dep.
             * The choice of fit and different scalings pre- or post- fit may have a large effect and should be studied.
             *
             * @param tot tot to convert
             * @return converted silicon energy
             */
            double convertTOT(const int tot) const;

        private:

            /** Digi Collection Name to use as input */
            std::string digiCollName_;

            /** Digi Pass Name to use as input */
            std::string digiPassName_;

            /** 
             * Layer Weights to use for this reconstruction 
             *
             * Layer weights account for the energy lost in the absorber directly
             * in front of the Silicon layer where the measured energy was deposited.
             * These are determined by calculating the average amount of energy lost
             * by a MIP passing through the extra material between sensitive layers.
             */
            std::vector<double> layerWeights_;

            /**
             * Second Order Energy Correction to use for this reconstruction
             *
             * This is a shift applied to all of the energies in order to have the
             * mean of the total energy deposited in the ECal be accurate.
             * This is less physically motivated than the layer weights and is more
             * of a calibration number.
             */
            double secondOrderEnergyCorrection_;

            /**
             * Helper Instance of EcalHexReadout:
             * performs real space x,y postion <-> module,cell ID translation
             */
            std::unique_ptr<EcalHexReadout> ecalHexReadout_;

            /**
             * Approximate energy deposited in Silicon layer for a MIP hit
             * MeV
             */
            static const double MIP_SI_RESPONSE; // MeV

    };
}

#endif
