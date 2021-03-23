
#include "Tools/HgcrocEmulator.h"

#include <cmath>

namespace ldmx {

HgcrocEmulator::HgcrocEmulator(const framework::config::Parameters &ps) {
  // settings of readout chip that are the same for all chips
  //  used  in actual digitization
  noiseRMS_ = ps.getParameter<double>("noiseRMS");
  timingJitter_ = ps.getParameter<double>("timingJitter");
  clockCycle_ = ps.getParameter<double>("clockCycle");
  totMax_ = ps.getParameter<double>("totMax");
  rateUpSlope_ = ps.getParameter<double>("rateUpSlope");
  timeUpSlope_ = ps.getParameter<double>("timeUpSlope");
  rateDnSlope_ = ps.getParameter<double>("rateDnSlope");
  timeDnSlope_ = ps.getParameter<double>("timeDnSlope");
  timePeak_ = ps.getParameter<double>("timePeak");
  nADCs_ = ps.getParameter<int>("nADCs");
  iSOI_ = ps.getParameter<int>("iSOI");
  noise_ = ps.getParameter<bool>("noise");
  readoutPadCapacitance_ = ps.getParameter<double>("readoutPadCapacitance");

  // conditions/settings of chip that may change between chips
  //  the ones passed here are the "defaults", i.e. if
  //  no extra conditions information is passed, then the emulator
  //  uses these parameters
  gain_ = ps.getParameter<double>("gain");
  pedestal_ = ps.getParameter<double>("pedestal");
  readoutThreshold_ = ps.getParameter<double>("readoutThreshold");
  toaThreshold_ = ps.getParameter<double>("toaThreshold");
  totThreshold_ = ps.getParameter<double>("totThreshold");
  measTime_ = ps.getParameter<double>("measTime");
  drainRate_ = ps.getParameter<double>("drainRate");

  // Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024. / clockCycle_;

  // Configure the pulse shape function
  pulseFunc_ =
      TF1("pulseFunc",
          "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/"
          "((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
          0.0, (double)nADCs_ * clockCycle_);
  pulseFunc_.FixParameter(1, rateUpSlope_);
  pulseFunc_.FixParameter(2, timeUpSlope_);
  pulseFunc_.FixParameter(3, timePeak_);
  pulseFunc_.FixParameter(5, rateDnSlope_);
  pulseFunc_.FixParameter(6, timeDnSlope_);
}

void HgcrocEmulator::seedGenerator(uint64_t seed) {
  noiseInjector_ = std::make_unique<TRandom3>(seed);
}

bool HgcrocEmulator::digitize(
    const int &channelID, const std::vector<double> &voltages,
    const std::vector<double> &times,
    std::vector<ldmx::HgcrocDigiCollection::Sample> &digiToAdd) const {

  /**
   * Construct Pulse Begin Digitized
   *
   *  This process is unfortunately complicated.
   *
   *  When the pulses are "close enough" to each other,
   *  they will be effectively merged into a single pulse
   *  of a higher magnitude. This merging is necessary for
   *  TOT mode because we need that total amplitude instead
   *  of only one of the constiuent amplitudes in order to
   *  calculate how much charge needs to be drained off.
   *
   *  When the pulses are "far enough" apart,
   *  they will NOT be merged into a single pulse and
   *  the shape changes to a multi-peaked wave form.
   *  If this multi-peaked wave never goes above the TOT
   *  threshold, then we just sample it at the sampling times.
   *
   *  If this mutli-peaked wave DOES go into TOT,
   *  then we need to disregard all pulses after the pulse
   *  (which could be a merged pulse) that triggered TOT
   *  to reflect the fact that the chip is in TOT-in-progress.
   *  But we also need to go back to ADC mode if TOT completes
   *  relatively quickly.
   *
   *  So. We need to merge "close enough" pulses into
   *  one pulse and we need to keep "far enough" pulses
   *  separate.
   *
   *  - How does the pre-amp shape several analog pulses coming together
   *    when they are separated by time? Do they just add linearly?
   */
  std::vector<std::pair<double,double>> analog_pulses;
  for (int i_sim{0}; i_sim < voltages.size(); i_sim++) {
    // merge with previously made analog pulses
    //  merges with first pulse found within 10ns of current sim pulse
    //  the value '10ns' is somewhat arbitrarily chosen
    bool merged{false};
    for (auto &[time, voltage] : analog_pulses) {
      if (abs(time-times.at(i_sim)) < 10.) {
        merged = true;
        time = (voltage*time + voltages.at(i_sim)*times.at(i_sim))/(voltage+voltages.at(i_sim));
        voltage += voltages.at(i_sim);
        break;
      }
    }

    // make new pulse if can't be merged with others
    if (not merged)
      analog_pulses.emplace_back(times.at(i_sim), voltages.at(i_sim));
  }

  /**
   * Now we are assuming that the pulses are effectively merged
   */

  // modify simulated TOA of pulses
  for (auto &[time, voltage] : analog_pulses) {
    // the peak of the pulse is ~13ns after the TOA
    // for a high-energy ADC Mode pulse
    //  this shift was determined by looking at graphs
    //  of the pulse shape
    time += 13.;

    // put noise onto timing
    // TODO more physical way of simulating the timing jitter
    if (noise_) time += noiseInjector_->Gaus(0., timingJitter_);
  }

  // make sure pulses are ordered by time of arrival
  std::sort(analog_pulses.begin(), analog_pulses.end(),
      [](const std::pair<double,double>& l, const std::pair<double,double>& r) {
        return (l.first < r.first);
        });

  /**
   * Find out if we go into TOT mode
   */
  int tot_mode_index{-1};
  for (int i_pulse{0}; i_pulse < analog_pulses.size(); i_pulse++) {
    if (analog_pulses.at(i_pulse).second > totThreshold_) {
      tot_mode_index = i_pulse;
      break;
    }
  }

  // Configure chip settings based off of table (that may have been passed)
  double gain = getCondition(channelID, "gain", gain_);
  double pedestal = getCondition(channelID, "pedestal", pedestal_);
  double toaThreshold = getCondition(channelID, "toaThreshold", toaThreshold_);
  double totThreshold = getCondition(channelID, "totThreshold", totThreshold_);
  double measTime = getCondition(channelID, "measTime", measTime_);
  double drainRate = getCondition(channelID, "drainRate", drainRate_);

  double readoutThresholdFloat =
      getCondition(channelID, "readoutThreshold", readoutThreshold_);
  int readoutThreshold = int(readoutThresholdFloat);
  /* debug printout
  std::cout << "Configuration: {"
            << "gain: " << gain << " mV/ADC, "
            << "pedestal: " << pedestal << ", "
            << "readout: " << readoutThreshold << ", "
            << "toa: " << toaThreshold << " mV, "
            << "tot: " << totThreshold << " mV, "
            << "measTime: " << measTime << " ns, "
            << "drainRate: " << drainRate << " mV/ns }"
            << std::endl;
   */

  auto measurePulse = [this, &gain, &pedestal](double time, bool withNoise) {
    auto signal = gain * pedestal + pulseFunc_.Eval(time);
    if (withNoise) signal += noiseInjector_->Gaus(0., noiseRMS_);
    return signal;
  };

  if (verbose_) {
    std::cout << "Input Pulse(s) [ns, mV] { ";
    for (const auto& [time, voltage] : analog_pulses)
      std::cout << "[ " << time << ", " << voltage << "] ";
    std::cout << "} -> ";
  }

  if (tot_mode_index < 0) {
    /**
     * ADC Readout Mode
     *
     * This is the simpler readout mode!
     * We just sample the voltage wave-form at
     * the specified measurement times for each
     * of our digi samples.
     */

    // below TOT threshold -> do ADC readout mode
    if (verbose_) std::cout << "ADC Mode { ";

    double toa{9999.};
    bool toa_was_measured{false};
    std::vector<double> voltage_measurements(nADCs_,0.);
    for (auto const &[time, voltage] : analog_pulses) {
      configurePulse(voltage, time);
      for (int i_adc{0}; i_adc < nADCs_; i_adc++) {
        double fullMeasTime = (i_adc - iSOI_)*clockCycle_ + measTime;
        voltage_measurements[i_adc] += measurePulse(fullMeasTime,noise_);        
      }

      // measure time of arrival (TOA) using TOA threshold
      // make sure pulse crosses TOA threshold
      if (voltage > toaThreshold) {
        /**
         * TF1::GetX requires a search range,
         *  since we know the pulse shape will only cross
         *  a horizontal line once between -infty and the peak
         *  (at t=time) we set the max of the search range
         *  to the peak time and the min to some large negative value.
         */
        double this_toa = pulseFunc_.GetX(toaThreshold - gain * pedestal,
                              -999., time);

        // TODO allow for multiple TOAs
        //  this logic just takes the earliest TOA measured
        //  basically assuming that the wave-form stays above
        //  the TOA threshold for the rest of the samples after
        //  it first crosses it
        if (not toa_was_measured or this_toa < toa) 
          toa = this_toa;
          toa_was_measured = true;
        }
      }

    if (verbose_) {
      std::cout << "TOA: ";
      if (toa_was_measured)
        std::cout << toa << "ns, ";
      else
        std::cout << "NA, ";
    }

    int toa_index{iSOI_};
    if (toa_was_measured) {
      toa_index = int(toa/clockCycle_)+iSOI_;
      toa = fmod(toa, clockCycle_);
    }

    // make sure the digi we are constructing is clean
    digiToAdd.clear();
    for (unsigned int i_adc{0}; i_adc < nADCs_; i_adc++) {
      int tdc{0}; // 0 == no TOA measurement
      if (i_adc == toa_index and toa_was_measured)
        tdc = toa*ns_ + 1; //plus one to offset from no-measurement case

      int adc_tm1{pedestal};
      if (i_adc > 0)
        adc_tm1 = digiToAdd.at(i_adc-1).adc_t();

      digiToAdd.emplace_back(
        false, false,  // use flags to mark this sample as an ADC measurement
        adc_tm1,
        voltage_measurements.at(i_adc)/gain,
        tdc
      );
      if (verbose_)
        std::cout << " ADC " << i_adc << ": " << digiToAdd[i_adc].adc_t() << ", ";

    }
    if (verbose_) std::cout << "}" << std::endl;

    // Determine whether to readout this hit depending on the readout threshold
    return (digiToAdd.at(iSOI_).adc_t() >= readoutThreshold);
  } else {
    /**
     * TOT Readout Mode
     *
     * The index of the pulse that pushed us over the threshold is
     * in tot_mode_index and we know that it is valid.
     */

    auto the_big_pulse = analog_pulses.at(tot_mode_index);

    /**
     * Calculate charge deposited at the beginning of TOT mode
     *
     * We are only considering the single pulse that pushed us
     * over the TOT threshold. Should we include any pulses
     * prior to this pulse?
     */
    double charge_deposited = the_big_pulse.second * readoutPadCapacitance_;

    // Measure Time Over Threshold (TOT) by using the drain rate.
    //   1. Use drain rate to see how long it takes for the charge to drain off
    //   2. Translate this into DIGI samples

    // Assume linear drain with slope drain rate:
    //      y-intercept = pulse amplitude
    //      slope       = drain rate
    //  ==> x-intercept = amplitude / rate
    // actual time over threshold using the real signal voltage amplitude
    double tot = charge_deposited / drainRate;

    // TODO how to do this?
    //  Right now, this is likely to yield a value outside the SOI
    //  which doesn't really make any sense
    configurePulse(the_big_pulse.second, the_big_pulse.first);
    double toa{pulseFunc_.GetX(toaThreshold - gain * pedestal, 
        -999., the_big_pulse.first)};
    // calculate the index that toa occurred
    int toa_index{int(the_big_pulse.first/clockCycle_)+iSOI_};
    // convert toa to the value within that index
    toa = fmod(toa,clockCycle_);

    // calculate the index that tot started on
    int tot_index{int(the_big_pulse.first/clockCycle_)+iSOI_};
    // calculate number of indices that tot would be in progress
    int num_whole_clocks = int(tot / clockCycle_);

    // calculate the TDC counts for this tot measurement
    //  internally, the chip uses 12 bits (2^12 = 4096)
    //  to measure a maximum of tot Max [ns]
    // wrapping is prevented in implemententation of
    //  HgcrocDigiCollection::Sample
    int tdc_counts = int(tot * 4096 / totMax_) + pedestal;

    if (verbose_) {
      std::cout << "TOT Mode { "
                << "TOA I:" << toa_index << ", "
                << "TOA: " << toa << " ns, "
                << "TOT I:" << tot_index << ", "
                << "TOT: " << tot << " ns, "
                << "TDC: " << tdc_counts << "}" << std::endl;
    }

    for (unsigned int i_adc = 0; i_adc < nADCs_; i_adc++) {
      bool tot_progress, tot_complete;
      int secon_measurement;
      if (i_adc == tot_index) {
        // the TOT is reported in the sample it started on
        secon_measurement = tdc_counts;
        tot_progress = false;
        tot_complete = true;
      } else if (i_adc > tot_index and i_adc <= tot_index+num_whole_clocks) {
        // TOT in progress
        secon_measurement = pedestal; // invalid ADCt measurement
        tot_progress = true;
        tot_complete = false;
      } else {
        // before TOT or after TOT complete
        //  TODO actually handle this case by measuring everything!
        secon_measurement = pedestal; // TODO invalid ADCt measurement
        tot_progress = false;
        tot_complete = false;
      }

      int toa_tdc{0}; //0 == no-measurement
      if (i_adc == toa_index)
        toa_tdc = toa*ns_+1; // shift by one to avoid non-measurement value

      digiToAdd.emplace_back(
          tot_progress, tot_complete,  // flags to mark type of sample
          i_adc > 0 ? digiToAdd.at(i_adc - 1).adc_t()
                    : pedestal,  // first measurement is ADC t-1
          secon_measurement,
          toa_tdc
      );
    } // loop through digi samples

    // Always readout TOT hits
    return true;
  }  // TOT or ADC Readout Mode

}  // HgcrocEmulator::digitize

}  // namespace ldmx
