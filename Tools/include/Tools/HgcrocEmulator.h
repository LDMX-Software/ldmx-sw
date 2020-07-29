
#ifndef TOOLS_HGCROCEMULATOR_H
#define TOOLS_HGCROCEMULATOR_H

#include "Tools/NoiseGenerator.h"
#include "Event/HgcrocDigiCollection.h"
#include "Event/SimCalorimeterHit.h"
#include "Framework/Parameters.h"

//----------//
//   ROOT   //
//----------//
#include "TRandom3.h"
#include "TF1.h"

namespace ldmx { 

    /**
     * @class HgcrocEmulator
     * @brief Emulate the digitization procedure performed by the HGCROC.
     *
     * @TODO Allow for pulse shape parameters to be configurable
     * @TODO Shift the pulse SOI arbitrarily
     * @TODO More realistic TOT emulation
     */
    class HgcrocEmulator { 

        public: 

            /** 
             * Constructor 
             *
             * Configures the chip emulator using the passed parameters.
             */
            HgcrocEmulator(const Parameters& ps);

            /** Destructor */
            ~HgcrocEmulator() { /* empty on purpose */ }

            /**
             * Digitize the signals from the simulated hits
             *
             * This is where the hefty amount of work is done.
             * This function is separated into two parts.
             *  1. Emulating the ROC response to the input simulated hits.
             *  2. Putting noise pulses in the empty channels.
             *
             * ## Part 1: Emulation
             * - Sum the energy deposits in the sim hit and energy-weight average
             *   the time of the hits
             * - Put noise on the time of the hit using timingJitter_
             * - Convert the simulated energy deposit [MeV] into a voltage that the ROC
             *   recieves [mV]. This is done by the parameter MeV_
             * - Configure the pulse to have the calculated voltage amplitude as its
             *   peak and the simulated hit time as the time of its peak [ns]
             * - Determine what readout mode the ROC will choose:
             *   - Amplitude < readoutThreshold_ : skip the hit
             *   - Amplitude < totThreshold_ : ADC Mode (described below)
             *   - Amplitude > totThreshold_ : TOT Mode (described below)
             *
             * @todo Allow for the user to choose a sample of interest (iSOI_)
             * other than zero. This should shift which sample the peak of
             * the pulse is placed in.
             *
             * #### ADC Mode
             * Here, we measure the height of the pulse once per clock cycle.
             * This leaves us with nADCs_ samples for each digitized hit.
             * The voltage measurements are converted to ADC counts using
             * the parameter gain_.
             *
             * The time of arrival (TOA) is zero unless the amplitude
             * is greater than toaThreshold_. Then the TOA is set to
             * the point the pulse crosses the toaThreshold_ with
             * respect to the current clock window.
             * The time measurements are converted to clock counts
             * using 2^10=1024 and clockCycle_.
             *
             * Both the tot_complete_ and tot_progress_ flags are set
             * to false for all the samples.
             *
             * #### TOT Mode
             * Here, we measure the time that the pulse is over totThreshold_.
             * We calculate the TOT by measuring the TOA by seeing when the
             * pulse crosses totThreshold before the peak and by measuring
             * time under threshold (TUT) by measuring when the pulse crosses
             * under totThreshold_ after the peak.
             *
             * @note In reality, the pulse shape changes drastically when
             * the chip electronics are saturated, so this is not a very
             * realistic method of digitization.
             *
             * @todo Improve the TOT method of digitization by incorporating
             * the more linear response when saturated.
             *
             * The TOT is then converted into the samples using the following
             * algorithm. Beginning at the first sample,
             * if TOT > clockCycle_ or TOT < 0. :
             *      insert an ADC sample where the tot_progress flag is set to true
             *      decrement TOT by clockCycle_
             * else :
             *      insert the TOT measurement
             *
             * #### Pulse Measurement
             * All "measurements" of the pulse use the lambda function measurePulse.
             * This function incorporate the pedestal_ and optionally includes noise
             * according to noiseRMS_.
             *
             * ## Part 2: Noise
             * The noise amplitudes are retrieved from the NoiseGenerator::generateNoiseHits
             * function using the number of empty channels. 
             * Then the time, sample index, and detector ID are randomly chosen
             * using noiseInjector_ and generateNoiseID().
             *
             * @param[in] simHits vector of SimCalorimeterHit that should be emulated
             * @return HgcrocDigiCollection full of digitized hits
             */
            HgcrocDigiCollection digitize( const std::vector<SimCalorimeterHit> &simHits ) const;

        private:

            /**
             * Generate a noise ID randomly
             *
             * This is hiding the difference in the ecal and hcal emulation.
             * In order to get a random detector ID within the subdetector,
             * we need to use some geometry parameters. Hopefully,
             * these geometry parameters can be pulled in from a
             * detector data service in the future, but right now,
             * they will have to be passed as parameters.
             *
             * @return a random detector ID number
             */
            int generateNoiseID() const;
        
        private:

            /// Verbosity, not configurable, only helpful in development
            bool verbose_{true};

            /// Time interval for chip clock [ns]
            double clockCycle_;

            /// gain setting of the chip [mV / ADC units]
            double gain_;

            /// base pedestal [ADC units]
            double pedestal_;

            /// Depth of ADC buffer. 
            int nADCs_; 

            /// Index for the Sample Of Interest in the list of digi samples 
            int iSOI_;

            /// Noise RMS [mV]
            double noiseRMS_; 

            /// Min threshold for reading out a channel [mV]
            double readoutThreshold_;

            /// Min threshold for measuring TOA [mV]
            double toaThreshold_;

            /// Min threshold for measuring TOT [mV]
            double totThreshold_;

            /// Jitter of timing mechanism in the chip [ns]
            double timingJitter_;

            /// Conversion from energy [MeV] to voltage [mV]
            double MeV_;

            /// Conversion from time [ns] to counts
            double ns_;

            /// Total Number of Readout Channels (for noise simulation)
            int nTotalChannels_;

            /// Number Ecal Layers (for noise simulation in ECal)
            int nEcalLayers_;

            /// Number of Modules Per Layer (for noise simulation in ECal)
            int nModulesPerLayer_;

            /// Number of Cells Per Module (for noise simulation in ECal)
            int nCellsPerModule_;

            /// Flag to determine how to simulate noise ID
            bool ecal_;

            /// Generates noise hits based off of number of cells that are not hit
            std::unique_ptr<NoiseGenerator> noiseGenerator_;

            /// Generates Gaussian noise on top of real hits
            std::unique_ptr<TRandom3> noiseInjector_;

            /**
             * Functional shape of signal pulse in time
             *
             * Shape parameters are hardcoded into the function currently.
             *  Pulse Shape: 
             *  [0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))
             *   p[0] = amplitude (height of peak in mV)
             *   p[1] = -0.345 shape parameter - rate of up slope
             *   p[2] = 70.6547 shape parameter - time of up slope relative to shape fit
             *   p[3] = 77.732 shape parameter - time of peak relative to shape fit
             *   p[4] = peak time (related to time of hit [ns])
             *   p[5] = 0.140068 shape parameter - rate of down slope
             *   p[6] = 87.7649 shape paramter - time of down slope relative to shape fit
             *
             * @f[
             *  V(t) = 
             *  p_0\frac{(1+\exp(p_1(-p_2+p_3)))(1+\exp(p_5*(-p_6+p_3)))}
             *          {(1+\exp(p_1(t-p_2+p_3-p_4)))(1+\exp(p_5*(t-p_6+p_3-p_4)))}
             * @f]
             */
            mutable TF1 pulseFunc_;

    }; // HgcrocEmulator

} // ldmx

#endif // TOOLS_HGCROCEMULATOR_H
