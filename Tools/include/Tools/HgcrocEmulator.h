
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
             * Set the function that will generate a noise ID given a set of already used IDs.
             */
            void setNoiseIDGenerator(int (*generator)(std::set<int>&)) { generateNoiseID_ = generator; }

            /**
             * Digitize the signals from the simulated hits
             *
             * @todo More realistic TOT simulation. Right now, it is using the basic
             * pulse shape, but we know that is _not_ how it works.
             *
             * @param[in] simHits vector of SimCalorimeterHit that should be emulated
             * @return HgcrocDigiCollection full of digitized hits
             */
            HgcrocDigiCollection digitize( const std::vector<SimCalorimeterHit> &simHits ) const;
        
        private:

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

            /// Pointer to noise-ID-generation function
            int (*generateNoiseID_)(std::set<int> &);

            /// Generates noise hits based off of number of cells that are not hit
            std::unique_ptr<NoiseGenerator> noiseGenerator_;

            /// Generates Gaussian noise on top of real hits
            std::unique_ptr<TRandom3> noiseInjector_;

            /**
             * Functional shape of signal pulse in time
             *
             * Shape parameters are hardcoded into the function currently.
             *  Pulse Shape:
                [0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))
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
            TF1 pulseFunc_;

    }; // HgcrocEmulator

} // ldmx

#endif // TOOLS_HGCROCEMULATOR_H
