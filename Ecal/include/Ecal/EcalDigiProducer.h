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
#include <time.h> //for initial seed to TRandom3
#include <memory> //for smart pointers
#include <map> //for rawID mapping

//----------//
//   ROOT   //
//----------//
#include "TRandom3.h"
#include "TF1.h"
#include "TH2F.h"

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
            virtual void configure(Parameters&);

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

            /**
             * Construct the DIGIs from energy and time simulated data.
             *
             * Right now, it adds up the energies and finds an
             * energy-weighted average of the times. These digis are
             * then inserted into digiToAdd.
             *
             * energies and times must be the same size.
             * NO CHECKING IS DONE FOR THIS.
             *
             * The ID of the digi samples is not set in this function.
             * It is only meant to make the energy+time --> digis translation consistent and isolated.
             *
             * @param energies simulated energy depositions
             * @param times simulated times of energy depositions
             * @param digiToAdd vector of EcalDigiSamples that will be filled with constructed digis
             * @return true if digiToAdd was actually filled with something
             */
            bool constructDigis(const std::vector<double> &energies, const std::vector<double> &times, std::vector<EcalDigiSample> &digiToAdd);


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
            double gain_;

            /** The pedestal in ADC units */
            double pedestal_;

            /** Set the noise (in electrons) when the capacitance is 0. */
            double noiseIntercept_;

            /** Set the capacitative noise slope (electrons/pF). */
            double noiseSlope_; 

            /** Capacitance per cell pad. */
            double padCapacitance_; // pF 

            /** Depth of ADC buffer. */
            int nADCs_; 

            /** Index for the Sample Of Interest in the list of digi samples */
            int iSOI_;

            /** Should we make and fill configuration histograms? */
            bool makeConfigHists_;

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
             * Generates noise hits based off of number of cells that are not hit
             */
            std::unique_ptr<NoiseGenerator> noiseGenerator_;

            /**
             * Generates Gaussian noise on top of real hits
             */
            std::unique_ptr<TRandom3> noiseInjector_;

            /**
             * Functional shape of signal pulse in time
             *
             * Shape parameters are hardcoded into the function currently.
             *  Pulse Shape:
             *   p[0]/(1.0+exp(p[1](t-p[2]+p[3]-p[4])))/(1.0+exp(p[5]*(t-p[6]+p[3]-p[4])))
             *   p[0] = amplitude (related to E dep through gain_)
             *   p[1] = -0.345 shape parameter - rate of up slope
             *   p[2] = 70.6547 shape parameter - time of up slope relative to shape fit
             *   p[3] = 77.732 shape parameter - time of peak relative to shape fit
             *   p[4] = peak time to be fit (related to time of hit [ns])
             *   p[5] = 0.140068 shape parameter - rate of down slope
             *   p[6] = 87.7649 shape paramter - time of down slope relative to shape fit
             */
            TF1 pulseFunc_;


            /**
             * Optional Histogram to be filled to help with configuring recon
             */
            TH2F *tot_SimE_{nullptr};
    };
}

#endif
