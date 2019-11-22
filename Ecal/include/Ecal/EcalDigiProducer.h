/**
 * @file EcalDigiProducer.h
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_ECALDIGIPRODUCER_H_
#define EVENTPROC_ECALDIGIPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <time.h>
#include <memory> //for smart pointers
#include <map> //for rawID mapping

//----------//
//   ROOT   //
//----------//
#include "TRandom3.h"
#include "TF1.h"

//----------//
//   LDMX   //
//----------//
#include "Event/EcalDigiCollection.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"

namespace ldmx {

    /**
     * @class EcalDigiProducer
     * @brief Performs basic ECal digitization
     */
    class EcalDigiProducer : public Producer {

        public:

            /**
             * Constructor
             * Makes unique noise generator and injector for this class
             */
            EcalDigiProducer(const std::string& name, Process& process);

            /**
             * Destructor
             * Deletes digi collection if it has been created
             */
            virtual ~EcalDigiProducer();

            /**
             * Configure this producer from the python configuration.
             * Sets event constants and configures the noise generator, noise injector, and pulse function.
             * Creates digi collection
             */
            virtual void configure(const ParameterSet&);

            /**
             * Simulates measurement of pulse and creates digi collection for input event.
             */
            virtual void produce(Event& event);

        private:

            /** 
             * Calculate the noise in electrons given the pad capacitance. 
             *
             * @param capacitance capacitance in pF
             * @return noise in electrons
             */
            double calculateNoise(const double capacitance, const double noiseIntercept, const double noiseSlope) { 
                return noiseIntercept + noiseSlope*capacitance;
            } 

            //Universal Constants
            //  Won't be changed by python configuration
            
            /** Electrons per MIP. */
            static const double ELECTRONS_PER_MIP; 

            /** MIP response in MeV. */
            static const double MIP_SI_RESPONSE;

            /** Time interval for chip clock */
            static const double CLOCK_CYCLE;

            /** Number of layers in ECal */
            static const int NUM_ECAL_LAYERS;

            /** Number of Hexagnonal modules per layer in ECal */
            static const int NUM_HEX_MODULES_PER_LAYER;

            /** Number of cells in each hex module */
            static const int CELLS_PER_HEX_MODULE;

            /** Total number of channels in entire ECal */
            static const int TOTAL_NUM_CHANNELS;

            //Python Configuration Parameters
            
            /** The gain in ADC units per MeV. */
            double gain_{2000.};

            /** The pedestal in ADC units */
            double pedestal_{1100.};

            /** Set the noise (in electrons) when the capacitance is 0. */
            double noiseIntercept_{900.};

            /** Set the capacitative noise slope (electrons/pF). */
            double noiseSlope_{22.}; 

            /** Capacitance per cell pad. */
            double padCapacitance_{27.56}; // pF 

            /** Depth of ADC buffer. */
            int nADCs_{10}; 

            //Member Variables that are used for each event

            /** 
             * Noise RMS in units of electrons. 
             * Calculated using the python config parameters and calculateNoise
             */
            double noiseRMS_{0}; 

            /** 
             * Set the threshold for reading out a channel.
             * Units are multiples of RMS noise for python config
             * Set to value of input config parameter times noiseRMS_
             */
            double readoutThreshold_{4.};

            /**
             * Helper Object that translates between realspace and cell/module IDs in the hexagonal layout
             */
            std::unique_ptr<EcalHexReadout> hexReadout_;

            /**
             * Generates noise hits based off of number of cells that are not hit
             */
            std::unique_ptr<NoiseGenerator> noiseGenerator_;

            /**
             * Generates Gaussian noise on top of real hits
             */
            std::unique_ptr<TRandom3> noiseInjector_;

            /**
             * Functional shape of signal pulse in time
             * Shape parameters are hardcoded into the function currently.
             * par[0] = time pulse reaches its peak
             * par[1] = amplitude of pulse at its peak
             */
            TF1 pulseFunc_;

            /**
             * Collection object that will be used as the reference for the event bus.
             */
            EcalDigiCollection *ecalDigis_;
    };
}

#endif
