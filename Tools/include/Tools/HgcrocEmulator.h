
#ifndef TOOLS_HGCROCEMULATOR_H
#define TOOLS_HGCROCEMULATOR_H

#include "Conditions/SimpleTableCondition.h"
#include "Framework/Configure/Parameters.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Tools/NoiseGenerator.h"

//----------//
//   ROOT   //
//----------//
#include "TF1.h"
#include "TRandom3.h"

namespace ldmx {

/**
 * @class HgcrocEmulator
 * @brief Emulate the digitization procedure performed by the HGCROC.
 *
 * This object emulates how the chip converts the analog signal
 * into DIGI samples. With that in mind, the digitize method
 * converts a set of voltages and times into the DIGI.
 *
 * This object _does not_ do everything related to subsystem information.
 * It does _not_ simulate noise within the empty channels,
 * and it does _not_ convert simulated energy depositions into
 * voltages. These tasks depend on the detector construction,
 * so they are left to the individual subsystem producers.
 *
 * @TODO time phase setting relative to target t=0ns using electronic IDs
 *
 * @TODO accurately model recovering from saturation (TOT Mode).
 * Currently, we just have all the samples after the sample triggering
 * TOT mode rail.
 */
class HgcrocEmulator {
 public:
  /**
   * Constructor
   *
   * Configures the chip emulator using the passed parameters.
   */
  HgcrocEmulator(const framework::config::Parameters& ps);

  /** Destructor */
  ~HgcrocEmulator() {}

  /**
   * Check if emulator has been seeded
   * @return true if random generator has been seeded
   */
  bool hasSeed() const { return noiseInjector_.get() != nullptr; }

  /**
   * Seed the emulator for random number generation
   * @param[in] seed integer to use as random seed
   */
  void seedGenerator(uint64_t seed);

  /**
   * Set Conditions
   *
   * Passes the chips conditions to be cached here and
   * used later in digitization.
   *
   * @param table conditions::DoubleTableConditions to be used for chip
   * parameters
   */
  void condition(const conditions::DoubleTableCondition& table) {
    // reset cache of column numbers if table changes
    if (&table != chipConditions_) conditionNamesToIndex_.clear();
    chipConditions_ = &table;
  }

  /**
   * Digitize the signals from the simulated hits
   *
   * This is where the hefty amount of work is done.
   *
   * 0. Prepare for Emulation
   *    - Clear input list of digi samples
   *    - Get conditions for the current chip
   *    - Sort the input sim voltage hits by amplitude
   *
   * 1. Combine input simulated hits into one CompositePulse to digitize.
   *    - This composite pulse decides whether to merge two simulated hits
   *      into one larger pulse depending on how close they are in time.
   *
   * 2. Add a timing jitter TODO
   *
   * 3. Go through sampling baskets one-by-one
   *    - If enter pulse goes above tot threshold, then enter TOT
   *      readout mode, digitize, and return true.
   *    - If pulse never goes above tot threshold, then only return
   *      true if the voltage sample taken in the SOI is above
   *      the readout threshold.
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
   * a linear drain of the charge off of the chip.
   *
   * The charge deposited on the chip is converted from the
   * pulse triggering TOT without including the pedestal.
   * The pedestal is included in the pulse when determining
   * if the pulse should enter TOT mode.
   *
   * Thus, TOT = charge deposited / drain rate
   * and TOA is calculated as before, seeing when the
   * pulse crossed the TOA threshold.
   *
   * #### Pulse Measurement
   * All "measurements" of the pulse use the object CompositePulse.
   * This function incorporates the pedestal_ and linearly adds all
   * the pulses at a variety of times. The pulses at different times
   * are "merged" upon addition to the composite pulse depending on
   * how close they are. This is done in a single pass, so we might
   * end up with pulses closer than the provided separation time.
   *
   * @note For more realism, some chip parameters should change depending on the
   * chip they are coming from. This should be modified here, with a package
   * of "HgcrocConditions" passed to this function to configure the emulator
   * before digitizing.
   *
   * @param[in] channelID raw integer ID for this readout channel
   * @param[in] arriving_pulses pairs of (voltage,time) of hits arriving at the chip
   * @param[out] digiToAdd digi that will be filled with the samples from the
   * chip
   * @return true if digis were constructed (false if hit was below readout)
   */
  bool digitize(
      const int& channelID, 
      std::vector<std::pair<double,double>>& arriving_pulses,
      std::vector<ldmx::HgcrocDigiCollection::Sample>& digiToAdd) const;

  /// Gain for input channel
  double gain(const int& channelID) const {
    return getCondition(channelID, "GAIN");
  }

  /// Pedestal [ADC Counts] for input channel
  double pedestal(const int& id) const {
    return getCondition(id, "PEDESTAL");
  }

  /// Readout Threshold (ADC Counts)
  double readoutThreshold(const int& id) const {
    return getCondition(id, "READOUT_THRESHOLD");
  }

 private:
  /**
   * Get condition for input chip ID, condition name, and default value
   *
   * @param[in] id chip global integer ID used in condition table
   * @param[in] name std::string name of chip parameter in table
   * @param[in] def default value for parameter if not found in table (or table
   * not set)
   * @return value of chip parameter
   */
  double getCondition(int id, const std::string& name) const {
    // check if emulator has been passed a table of conditions
    if (!chipConditions_) {
      EXCEPTION_RAISE("HgcrocCond",
          "HGC ROC Emulator was not given a conditions table.");
    }

    // cache column index for the input name
    if (conditionNamesToIndex_.count(name) == 0)
      conditionNamesToIndex_[name] = chipConditions_->getColumnNumber(name);

    // get condition
    return chipConditions_->get(id, conditionNamesToIndex_.at(name));
  }

 private:
  /**
   * CompositePulse
   *
   * An emulator for a pulse that the chip needs to read.
   * This handles merging two hits that are "close-enough"
   * to one another.
   */
  class CompositePulse {
   public:
    /**
     * Constructore
     *
     * Connect this pulse emulator with the pulse
     * shape function already configured by the chip
     * emulator.
     */
    CompositePulse(TF1& func, const double& g, const double& p) 
      : pulseFunc_{func}, gain_{g}, pedestal_{p} { }
  
    /**
     * Put another hit into this composite pulse.
     *
     * If the hit is within the merge input of a hit already
     * included, then it is merged with that hit. Otherwise,
     * it is included as its own hit.
     *
     * @param[in] hit voltage,time pair representing a sime hit
     * @param[in] hit_merge_ns maximum time separation [ns] to merge two hits
     */
    void addOrMerge(const std::pair<double,double>& hit, double hit_merge_ns) {
      auto imerge{hits_.begin()};
      for (; imerge!=hits_.end(); imerge++) 
        if (fabs(imerge->second-hit.second)<hit_merge_ns) break;
      if (imerge == hits_.end()) { // didn't find a match, add to the list
        hits_.push_back(hit);
      } else { // merge hits, shifting time to average
        imerge->second=(imerge->second*imerge->first+hit.first*hit.second);
        imerge->first+=hit.first;
        imerge->second/=imerge->first;
      }
    }
  
    /**
     * Find the time at which we cross the input level.
     *
     * We use the midpoint algorithm, assuming the input low
     * is below the threshold and hight is above.
     *
     * @param[in] low minimum value (below threshold) to start search at [mV]
     * @param[in] high maximum value (above threshold) to start search at [mV]
     * @param[in] level threshold to look for time [mV]
     * @param[in] prec precision with which to look [mV]
     * @returns time [ns] at which the pulse cross level
     */
    double findCrossing(double low, double high, double level, double prec=0.01) {
      // use midpoint algorithm, assumes low is below and high is above
      double step=high-low;
      double pt=(high+low)/2;
      while (step>prec) {
        double vmid=at(pt);
        if (vmid<level) {
          low=pt;
        } else {
          high=pt;
        }
        step=high-low;
        pt=(high+low)/2;
      }
      return pt;
    }
  
    /// Configure the pulses for the current chip
    void setGainPedestal(double gain, double pedestal) {
      gain_=gain;
      pedestal_=pedestal;
    }
  
    /**
     * Evaluating this object as a function
     * gives the same result as at.
     *
     * @see at
     */
    double operator()(double time) const {
      return at(time);
    }
  
    /**
     * Measure the voltage at the input time
     *
     * Includes the effects from all pulses but
     * does not put any noise into the measurement.
     *
     * @param[in] time time to measure [ns]
     * @return voltage at that time [mV]
     */
    double at(double time) const {
      double signal = gain_ * pedestal_;
      for (auto hit : hits_)
        signal += hit.first * pulseFunc_.Eval(time-hit.second);
      return signal;
    };
  
    /// Get list of individual pulses that are entering the chip
    const std::vector<std::pair<double,double>>& hits() const { return hits_; }
    
   private:
    /**
     * pulses entering the chip
     *
     * The pair is {voltage amplitude [mV], time of peak [ns]}
     */
    std::vector<std::pair<double,double>> hits_;

    /// gain for current chip we are emulating
    double gain_;

    /// pedestal for current chip we are emulating
    double pedestal_;

    /// reference to pulse shape function shared by all pulses
    TF1& pulseFunc_;
    
  };  // CompositePulse

 private:
  /**************************************************************************************
   * Parameters Identical for all Chips
   *************************************************************************************/

  /// Put noise in channels, only configure to false if testing
  bool noise_{true};

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

  /// Hit merging time [ns]
  double hit_merge_ns_;

  /**************************************************************************************
   * Chip-Dependent Parameters (Conditions)
   *************************************************************************************/

  /**
   * Handle to table of chip-dependent conditions
   *
   * The defaults are listed below and are separate parameters
   * passed through the python configuration.
   */
  const conditions::DoubleTableCondition* chipConditions_{nullptr};

  /**
   * Map of condition names to column numbers
   *
   * mutable so that we can update the cached column values
   * in getCondition during processing.
   */
  mutable std::map<std::string, int> conditionNamesToIndex_;

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

};  // HgcrocEmulator

}  // namespace ldmx

#endif  // TOOLS_HGCROCEMULATOR_H
