
#include "Tools/HgcrocEmulator.h"

namespace ldmx {

HgcrocEmulator::HgcrocEmulator(const framework::config::Parameters &ps) {
  // settings of readout chip that are the same for all chips
  //  used  in actual digitization
  noise_ = ps.get<bool>("noise");
  timing_jitter_ = ps.get<double>("timingJitter");
  rate_up_slope_ = ps.get<double>("rateUpSlope");
  time_up_slope_ = ps.get<double>("timeUpSlope");
  rate_dn_slope_ = ps.get<double>("rateDnSlope");
  time_dn_slope_ = ps.get<double>("timeDnSlope");
  time_peak_ = ps.get<double>("timePeak");
  clock_cycle_ = ps.get<double>("clockCycle");
  n_ad_cs_ = ps.get<int>("nADCs");
  i_soi_ = ps.get<int>("iSOI");

  // Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024. / clock_cycle_;

  hit_merge_ns_ = 0.05;  // combine at 50 ps level

  // Configure the pulse shape function
  pulse_func_ =
      TF1("pulseFunc",
          "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/"
          "((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
          0.0, (double)n_ad_cs_ * clock_cycle_);
  pulse_func_.FixParameter(0, 1.0);  // amplitude is set externally
  pulse_func_.FixParameter(1, rate_up_slope_);
  pulse_func_.FixParameter(2, time_up_slope_);
  pulse_func_.FixParameter(3, time_peak_);
  pulse_func_.FixParameter(4, 0);  // not using time offset in this way
  pulse_func_.FixParameter(5, rate_dn_slope_);
  pulse_func_.FixParameter(6, time_dn_slope_);
}

void HgcrocEmulator::seedGenerator(uint64_t seed) {
  noise_injector_ = std::make_unique<TRandom3>(seed);
}

bool HgcrocEmulator::digitize(
    const int &channelID,
    std::vector<std::pair<double, double>> &arriving_pulses,
    std::vector<ldmx::HgcrocDigiCollection::Sample> &digiToAdd) const {
  // step 0: prepare ourselves for emulation
  digiToAdd.clear();  // make sure it is clean

  // Configure chip settings based off of table (that may have been passed)
  double tot_max = getCondition(channelID, "TOT_MAX");
  double pad_capacitance = getCondition(channelID, "PAD_CAPACITANCE");
  double gain = this->gain(channelID);
  double pedestal = this->pedestal(channelID);
  double toa_threshold = getCondition(channelID, "TOA_THRESHOLD");
  double tot_threshold = getCondition(channelID, "TOT_THRESHOLD");
  // measTime defines the point in the BX where an in-time
  //  (time=0 in times vector) hit would arrive.
  // Used to determine BX boundaries and TOA behavior.
  double meas_time = getCondition(channelID, "MEAS_TIME");
  double drain_rate = getCondition(channelID, "DRAIN_RATE");
  double readout_threshold_float = this->readoutThreshold(channelID);
  int readout_threshold = int(readout_threshold_float);

  // sort by amplitude
  //  ==> makes sure that puleses are merged towards higher ones
  std::sort(
      arriving_pulses.begin(), arriving_pulses.end(),
      [](const std::pair<double, double> &a,
         const std::pair<double, double> &b) { return a.first > b.first; });

  // step 1: gather voltages into groups separated by (programmable) ns, single
  // pass
  ldmx::CompositePulse pulse(pulse_func_, gain, pedestal);

  for (auto hit : arriving_pulses) pulse.addOrMerge(hit, hit_merge_ns_);

  // TODO step 2: add timing jitter
  // if (noise_) pulse.jitter();

  /// the time here is nominal (zero gives peak if hit.second is zero)

  // step 3: go through each BX sample one by one
  bool was_toa = false;
  for (int i_adc = 0; i_adc < n_ad_cs_; i_adc++) {
    double start_bx = (i_adc - i_soi_) * clock_cycle_ - meas_time;
    ldmx_log(trace) << "  iADC = " << i_adc << " at startBX = " << start_bx;

    // step 3b: check each merged hit to see if it peaks in this BX.  If so,
    // check its peak time to see if it's over TOT or TOA.
    bool start_tot = false;
    bool over_toa = false;
    double tover_toa = -1;
    double tover_tot = -1;
    for (auto hit : pulse.hits()) {
      int hit_bx = int((hit.second + meas_time) / clock_cycle_ + i_soi_);
      // if this hit wasn't in the current BX, continue...
      if (hit_bx != i_adc) {
        continue;
      }

      double vpeak = pulse(hit.second);

      if (vpeak > tot_threshold) {
        start_tot = true;
        // use the latest time in the window
        if (tover_tot < hit.second) {
          tover_tot = hit.second;
        }
      }

      if (vpeak > toa_threshold) {
        if (!over_toa || hit.second < tover_toa) tover_toa = hit.second;
        over_toa = true;
      }

    }  // loop over sim hits_

    // check for the case of a TOA even though the peak is in the next BX
    if (!over_toa && pulse(start_bx + clock_cycle_) > toa_threshold) {
      if (pulse(start_bx) < toa_threshold) {
        // pulse crossed TOA threshold somewhere between the start of this
        // basket and the end
        over_toa = true;
        tover_toa = start_bx + clock_cycle_;
      }
    }

    if (start_tot) {
      // above TOT threshold -> do TOT readout mode

      // @TODO NO NOISE
      //  CompositePulse includes pedestal, we need to remove it
      //  when calculating the charge deposited.
      double charge_deposited =
          (pulse(tover_tot) - gain * pedestal) * pad_capacitance;

      // Measure Time Over Threshold (TOT) by using the drain rate.
      //  1. Use drain rate to see how long it takes for the charge to drain off
      //  2. Translate this into DIGI samples

      // Assume linear drain with slope drain rate:
      //      y_-intercept = pulse amplitude
      //      slope       = drain rate
      //  ==> x-intercept = amplitude / rate
      // actual time over threshold using the real signal voltage amplitude
      double tot = charge_deposited / drain_rate;
      ldmx_log(trace) << "    we are in TOT read-out mode, TOT = " << tot;

      // calculate the TDC counts for this tot measurement
      //  internally, the chip uses 12 bits (2^12 = 4096)
      //  to measure a maximum of tot Max [ns]
      int tdc_counts = int(tot * 4096 / tot_max) + pedestal;

      // were we already over TOA?  TOT is reported in BX where TOA went over
      // threshold...
      int toa{0};
      if (was_toa) {
        // TOA was in the past
        toa = digiToAdd.back().toa();
      } else {
        // TOA is here and we need to find it
        double timecross =
            pulse.findCrossing(start_bx, tover_tot, toa_threshold);
        toa = int((timecross - start_bx) * ns_);
        // keep inside valid limits
        if (toa == 0) toa = 1;
        if (toa > 1023) toa = 1023;
      }
      ldmx_log(trace) << "    Adding TOT hit with toa = " << toa
                      << ", tdc_counts = " << tdc_counts
                      << " adcT at prev iADC = "
                      << digiToAdd.at(i_adc - 1).adcT();
      // ADC at t-1
      auto adc_at_tminus1 =
          (i_adc > 0) ? digiToAdd.at(i_adc - 1).adcT() : pedestal;
      auto i_tot_sample = digiToAdd.size();
      // mark as a TOT measurement with 2nd boolean as true
      digiToAdd.emplace_back(false, true, adc_at_tminus1, tdc_counts, toa);

      // TODO: properly handle saturation and recovery, eventually.
      // Now just kill everything...
      ldmx_log(trace)
          << "   Adding further hits_ with ADC [t-1] = 0x3FF, toa = "
             "0x3FF, until digiToAdd.size() = "
          << digiToAdd.size() << " < n_ad_cs_(" << n_ad_cs_ << ")";
      while (digiToAdd.size() < n_ad_cs_) {
        // flags to mark type of sample
        digiToAdd.emplace_back(true, false, 0x3FF, 0x3FF, 0);
      }

      if (save_pulse_truth_info_){
        pulse_truth_coll_->push_back(ldmx::HgcrocPulseTruth(channelID, pulse));
      }

      // Read out if the toa is within one Bx after nominal
      return (i_tot_sample <= i_soi_ + 1);
    } else {
      // determine the voltage at the sampling time
      double bxvolts = pulse((i_adc - i_soi_) * clock_cycle_ - meas_time);
      // add noise if requested
      if (noise_) bxvolts += noise(channelID);
      // convert to integer and keep in range (handle low and high saturation)
      int adc = bxvolts / gain;
      ldmx_log(trace) << "    we are in ADC read-out mode, adc = " << adc;
      if (adc < 0) adc = 0;
      if (adc > 1023) adc = 1023;

      // check for TOA
      int toa(0);
      if (pulse(start_bx) < toa_threshold && over_toa) {
        double timecross =
            pulse.findCrossing(start_bx, tover_toa, toa_threshold);
        toa = int((timecross - start_bx) * ns_);
        // keep inside valid limits
        if (toa == 0) toa = 1;
        if (toa > 1023) toa = 1023;
        was_toa = true;
      } else {
        was_toa = false;
      }
      // ADC at t-1
      auto adc_t_minus1 =
          (i_adc > 0) ? digiToAdd.at(i_adc - 1).adcT() : pedestal;

      digiToAdd.emplace_back(false, false, adc_t_minus1, adc, toa);
    }  // TOT or ADC Mode
  }  // sampling baskets

  if (save_pulse_truth_info_){
    pulse_truth_coll_->push_back(ldmx::HgcrocPulseTruth(channelID, pulse));
  }

  // we only get here if we never went into TOT mode
  // check the SOI to see if we should read out
  ldmx_log(trace) << "  we are adding the hit IFF iSOI= " << i_soi_
                  << "'s adc_t = " << digiToAdd.at(i_soi_).adcT()
                  << " >= thresh (" << readout_threshold << ")";
  return digiToAdd.at(i_soi_).adcT() >= readout_threshold;
}  // HgcrocEmulator::digitize

std::vector<ldmx::HgcrocDigiCollection::Sample> HgcrocEmulator::noiseDigi(
    const int &channel, const double &soi_amplitude) const {
  // get chip conditions from emulator
  double pedestal{this->pedestal(channel)};
  double gain{this->gain(channel)};
  // fill a digi with noise samples
  std::vector<ldmx::HgcrocDigiCollection::Sample> noise_digi;
  for (int i_adc{0}; i_adc < n_ad_cs_; i_adc++) {
    // gen noise for ADC samples
    // ADC at t-1
    int adc_tm1{static_cast<int>(pedestal)};
    if (i_adc > 0) {
      adc_tm1 = noise_digi.at(i_adc - 1).adcT();
    } else {
      adc_tm1 += noise(channel) / gain;
    }
    // ADC at t
    int adc_t{static_cast<int>(pedestal + noise(channel) / gain)};

    if (i_adc == i_soi_) adc_t += soi_amplitude / gain;

    // set toa to 0 (not determined)
    // put new sample into noise digi
    noise_digi.emplace_back(false, false, adc_tm1, adc_t, 0);
  }  // samples in noise digi
  return noise_digi;
}

}  // namespace ldmx
