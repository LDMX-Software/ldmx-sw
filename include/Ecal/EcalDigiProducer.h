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
#include "Event/HgcrocDigiCollection.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/EcalID.h"
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
             * @param digiToAdd vector of HgcrocDigiCollection::Sample that will be filled with constructed digis
             * @return true if digiToAdd was actually filled with something
             */
            bool constructDigis(const std::vector<double> &energies, const std::vector<double> &times, 
                    std::vector<HgcrocDigiCollection::Sample> &digiToAdd);

        private:

            ///////////////////////////////////////////////////////////////////////////////////////
            //Python Configuration Parameters
            
            /// Number of Electrons Generated per MIP in 0.5mm thick Si
            double ELECTRONS_PER_MIP; 

            /// MIP response in Si in MeV.
            double MIP_SI_RESPONSE;

            /// Time interval for chip clock in ns
            double clockCycle_;

            /// Number of layers in ECal 
            int NUM_ECAL_LAYERS;

            /// Number of Hexagnonal modules per layer in ECal 
            int NUM_HEX_MODULES_PER_LAYER;

            /// Number of cells in each hex module 
            int CELLS_PER_HEX_MODULE;

            /// The gain in ADC units per MeV. 
            double gain_;

            /// The pedestal in ADC units 
            double pedestal_;

            /// Capacitance of readout pads in pF
            double readoutPadCapacitance_;

            /// Max ADC Count Setting [fC]
            double maxADC_;

            /// Depth of ADC buffer. 
            int nADCs_; 

            /// Index for the Sample Of Interest in the list of digi samples 
            int iSOI_;

            /// Should we make and fill configuration histograms? 
            bool makeConfigHists_;

            /// Noise RMS in MeV
            double noiseRMS_; 

            /// Min threshold for reading out a channel in MeV
            double readoutThreshold_;

            /// Min threshold for measuring TOA in MeV
            double toaThreshold_;

            /// Min threshold for measuring TOT in MeV
            double totThreshold_;

            /// Jitter of timing mechanism in the chip [ns]
            double timingJitter_;

            ///////////////////////////////////////////////////////////////////////////////////////
            // Other member variables

            /// Total number of channels in the ECal
            int TOTAL_NUM_CHANNELS;

            /// Conversion from time in ns to ticks of the internal clock
            double ns_;

            /// Conversion from energy in MeV to voltage in mV
            double MeV_;

            /// Conversion from mV to discrete ADC counts
            double mV_;

            /// Generates noise hits based off of number of cells that are not hit
            std::unique_ptr<NoiseGenerator> noiseGenerator_;

            /// Generates Gaussian noise on top of real hits
            std::unique_ptr<TRandom3> noiseInjector_;

            /**
             * Functional shape of signal pulse in time
             *
             * Shape parameters are hardcoded into the function currently.
             *  Pulse Shape:
             *   p[0]/(1.0+exp(p[1](t-p[2]+p[3]-p[4])))/(1.0+exp(p[5]*(t-p[6]+p[3]-p[4])))
             *   p[0] = amplitude (related to num electrons through gain_)
             *   p[1] = -0.345 shape parameter - rate of up slope
             *   p[2] = 70.6547 shape parameter - time of up slope relative to shape fit
             *   p[3] = 77.732 shape parameter - time of peak relative to shape fit
             *   p[4] = peak time to be fit (related to time of hit [ns])
             *   p[5] = 0.140068 shape parameter - rate of down slope
             *   p[6] = 87.7649 shape paramter - time of down slope relative to shape fit
             */
            TF1 pulseFunc_;
    };
}

#endif
