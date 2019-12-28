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
#include "TRandom3.h"

//----------//
//   LDMX   //
//----------//
#include "Event/EventDef.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"

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
            virtual void configure(const ParameterSet&);

            /**
             * Produce EcalHits and put them into the event bus using the
             * EcalDigis as input.
             */
            virtual void produce(Event& event);

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
             * Helper Instance of EcalDetectorID:
             * used to translate the raw ID to layer, module, cell IDs
             */
            EcalDetectorID detID_;

            /**
             * Helper Instance of EcalHexReadout:
             * performs real space x,y postion <-> module,cell ID translation
             */
            std::unique_ptr<EcalHexReadout> ecalHexReadout_;

            /**
             * Default Vector of Layer Weights
             * Used to convert from energy deposited in Silicon to approximate
             * energy from the particle deposited in absorber in front of the given layer.
             */
            static const std::vector<double> DEFAULT_LAYER_WEIGHTS;

            /**
             * Default Second Order Correction
             * Used to shift all the layer weights towards having the correct mean
             * for total energy deposited in ECal.
             */
            static const double DEFAULT_SECOND_ORDER_CORRECTION;

            /**
             * Approximate energy deposited in Silicon layer for a MIP hit
             * MeV
             */
            static const double MIP_SI_RESPONSE; // MeV

    };
}

#endif
