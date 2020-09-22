
#ifndef TOOLS_HGCROCEMULATOR_H
#define TOOLS_HGCROCEMULATOR_H

#include "Tools/NoiseGenerator.h"
#include "Event/HgcrocDigiCollection.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Framework/Configure/Parameters.h"
#include "Conditions/SimpleTableCondition.h"

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
     * This object emulates how the chip converts the analog signal
     * into DIGI samples. With that in mind, the digitize method
     * converts a set of voltages and times into the DIGI.
     *
     * This object _does not_ do anything related to subsystem information.
     * It does _not_ set the detector ID for the DIGI it constructs,
     * it does _not_ simulate noise within the empty channels,
     * and it does _not_ convert simulated energy depositions into
     * voltages. These tasks depend on the detector construction,
     * so they are left to the individual subsystem producers.
     *
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
             * Set Conditions
             *
             * Passes the chips conditions to be cached here and
             * used later in digitization.
             *
             * @param table DoubleTableConditions to be used for chip parameters
             */
            void condition(const DoubleTableCondition& table) {
                //reset cache of column numbers if table changes
                if(&table!=chipConditions_) conditionNamesToIndex_.clear();
                chipConditions_ = &table;
            }

            /**
             * Digitize the signals from the simulated hits
             *
             * This is where the hefty amount of work is done.
             *
             * - Sum the voltages and voltage-weight average the times
             * - Put noise on the time of the hit using timingJitter_
             * - Configure the pulse to have the calculated voltage amplitude as its
             *   peak and the simulated hit time as the time of its peak [ns]
             * - Determine what readout mode the ROC will choose:
             *   - Amplitude < readoutThreshold_ : skip the hit, return false
             *   - Amplitude < totThreshold_ : ADC Mode (described below)
             *   - Amplitude > totThreshold_ : TOT Mode (described below)
             *
             * @TODO Allow for the user to choose a sample of interest (iSOI_)
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
             * Here, we measure how long the chip is in saturation.
             * This is calculated using the drain rate and assuming
             * a linear drain of the voltage off of the chip.
             *
             * Thus, TOT = pulse peak / drain rate
             * and TOA is calculated as before, seeing when the 
             * pulse crossed the TOA threshold.
             *
             * @TODO What do the ADC measurements look like during the TOT measurment?
             *
             * The TOT is then converted into the samples using the following algorithm.
             *
             *  1. Calculate the number of clock cycles it would take for the TOT to be measured
             *  2. Convert the TOT measurement [ns] to TDC counts using totMax and the internal 12 bits
             *  3. Convert TDC counts (12 bits) to the 10 bit sample using
             *     - if ( TDC > 512 ) ten_bit_sample = TDC
             *     - else ten_bit_sample = 512 + TDC/8
             *  4. Insert the ten_bit_sample into the correct sample using the index above
             *  5. Set the other samples to normal ADC measurements, 
             *     using the flags to show if TOT is in progress or not
             *
             * #### Pulse Measurement
             * All "measurements" of the pulse use the member function measurePulse.
             * This function incorporate the pedestal_ and optionally includes noise
             * according to noiseRMS_.
             *
             * @note For more realism, some chip parameters should change depending on the
             * chip they are coming from. This should be modified here, with a package
             * of "HgcrocConditions" passed to this function to configure the emulator
             * before digitizing.
             *
             * @param[in] channelID raw integer ID for this readout channel
             * @param[in] voltages list of voltage amplitudes going into the chip
             * @param[in] times list of times corresponding to those voltage amplitudes
             * @param[out] digiToAdd digi that will be filled with the samples from the chip
             * @return true if digis were constructed (false if hit was below readout)
             */
            bool digitize( const int &channelID,
                    const std::vector<double> &voltages, 
                    const std::vector<double> &times, 
                    std::vector<HgcrocDigiCollection::Sample> &digiToAdd ) const;
        
        private:

            /**
             * Configure the pulse to match the input voltage peak and time.
             *
             * @param[in] amplitude voltage amplitude of pulse [mV]
             * @param[in] time time of peak [ns]
             */
            void configurePulse(double amplitude, double time) const {
                pulseFunc_.SetParameter( 0 , amplitude ); 
                pulseFunc_.SetParameter( 4 , time );
            }

            /**
             * Get condition for input chip ID, condition name, and default value
             * 
             * @param[in] id chip global integer ID used in condition table
             * @param[in] name std::string name of chip parameter in table
             * @param[in] def default value for parameter if not found in table (or table not set)
             * @return value of chip parameter
             */
            double getCondition(int id, const std::string& name, double def) const {
                //check if emulator has been passed a table of conditions
                if (!chipConditions_) return def;
                if ( conditionNamesToIndex_.count(name) == 0 ) 
                    conditionNamesToIndex_[name] = chipConditions_->getColumnNumber(name);
                double condition{def};
                try {
                    condition = chipConditions_->get(id,conditionNamesToIndex_.at(name));
                } catch(Exception&) {
                    //ignore thrown exceptions and use default instead
                    return def;
                }
                return condition;
            }

        private:

            /// Verbosity, not configurable, only helpful in development
            bool verbose_{false};

            /// Put noise in channels, not configurable, only helpful in development
            bool noise_{true};

            /**************************************************************************************
             * Parameters Identical for all Chips
             *************************************************************************************/

            /// Depth of ADC buffer. 
            int nADCs_; 

            /// Index for the Sample Of Interest in the list of digi samples 
            int iSOI_;

            /// Noise RMS [mV]
            double noiseRMS_; 

            /// Time interval for chip clock [ns]
            double clockCycle_;

            /// Jitter of timing mechanism in the chip [ns]
            double timingJitter_;

            /// Maximum TOT measured by chip [ns]
            double totMax_;

            /// Conversion from time [ns] to counts
            double ns_;

            /// Rate of Up Slope in Pulse Shape [1/ns]
            double rateUpSlope_;

            /// Time of Up Slope relative to Pulse Shape Fit [ns]
            double timeUpSlope_;

            /// Rate of Down Slope in Pulse Shape [1/ns]
            double rateDnSlope_;

            /// Time of Down Slope relative to Pulse Shape Fit [ns]
            double timeDnSlope_;
            
            /// Time of Peak relative to pulse shape fit [ns]
            double timePeak_;

            /**************************************************************************************
             * Chip-Dependent Parameters (Conditions)
             *************************************************************************************/

            /**
             * Handle to table of chip-dependent conditions
             *
             * The defaults are listed below and are separate parameters
             * passed through the python configuration.
             */
            const DoubleTableCondition* chipConditions_{nullptr};

            /**
             * Map of condition names to column numbers
             *
             * mutable so that we can update the cached column values
             * in getCondition during processing.
             */
            mutable std::map<std::string,int> conditionNamesToIndex_;

            /// gain setting of the chip [mV / ADC units]
            double gain_;

            /// base pedestal [ADC units]
            double pedestal_;

            /// Min threshold for reading out a channel [mV]
            double readoutThreshold_;

            /// Min threshold for measuring TOA [mV]
            double toaThreshold_;

            /// Min threshold for measuring TOT [mV]
            double totThreshold_;

            /// Measurement time relative to clock cycle [ns]
            double measTime_;

            /// Rate that charge drains off HGC ROC after being saturated [mV/ns]
            double drainRate_;

            /**************************************************************************************
             * Helpful Member Objects
             *************************************************************************************/

            /// Generates Gaussian noise on top of real hits
            std::unique_ptr<TRandom3> noiseInjector_;

            /**
             * Functional shape of signal pulse in time
             *
             * Shape parameters are hardcoded into the function currently.
             *  Pulse Shape: 
             *  [0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))
             *   p[0] = amplitude (height of peak in mV)
             *   p[1] = rate of up slope - rateUpSlope_
             *   p[2] = time of up slope relative to shape fit - timeUpSlope_
             *   p[3] = time of peak relative to shape fit - timePeak_
             *   p[4] = peak time (related to time of hit [ns])
             *   p[5] = rate of down slope - rateDnSlope_
             *   p[6] = time of down slope relative to shape fit - timeDnSlope_
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
