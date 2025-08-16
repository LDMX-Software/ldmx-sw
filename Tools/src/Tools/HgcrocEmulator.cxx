
#include "Tools/HgcrocEmulator.h"

namespace ldmx {

HgcrocEmulator::HgcrocEmulator(const framework::config::Parameters &ps) {
  // settings of readout chip that are the same for all chips
  //  used  in actual digitization
  noise_ = ps.get<bool>("noise");
  timingJitter_ = ps.get<double>("timingJitter");
  rateUpSlope_ = ps.get<double>("rateUpSlope");
  timeUpSlope_ = ps.get<double>("timeUpSlope");
  rateDnSlope_ = ps.get<double>("rateDnSlope");
  timeDnSlope_ = ps.get<double>("timeDnSlope");
  timePeak_ = ps.get<double>("timePeak");
  clock_cycle_ = ps.get<double>("clockCycle");
  nADCs_ = ps.get<int>("nADCs");
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
          0.0, (double)nADCs_ * clock_cycle_);
  pulse_func_.FixParameter(0, 1.0);  // amplitude is set externally
  pulse_func_.FixParameter(1, rateUpSlope_);
  pulse_func_.FixParameter(2, timeUpSlope_);
  pulse_func_.FixParameter(3, timePeak_);
  pulse_func_.FixParameter(4, 0);  // not using time offset in this way
  pulse_func_.FixParameter(5, rateDnSlope_);
  pulse_func_.FixParameter(6, timeDnSlope_);
}

void HgcrocEmulator::seedGenerator(uint64_t seed) {
  noiseInjector_ = std::make_unique<TRandom3>(seed);
}

bool HgcrocEmulator::digitize(
    const int &channelID,
    std::vector<std::pair<double, double>> &arriving_pulses,
    std::vector<ldmx::HgcrocDigiCollection::Sample> &digiToAdd) const {
  // step 0: prepare ourselves for emulation
  digiToAdd.clear();  // make sure it is clean

  // Configure chip settings based off of table (that may have been passed)
  double totMax = getCondition(channelID, "TOT_MAX");
  double padCapacitance = getCondition(channelID, "PAD_CAPACITANCE");
  double gain = this->gain(channelID);
  double pedestal = this->pedestal(channelID);
  double toaThreshold = getCondition(channelID, "TOA_THRESHOLD");
  double totThreshold = getCondition(channelID, "TOT_THRESHOLD");
  // measTime defines the point in the BX where an in-time
  //  (time=0 in times vector) hit would arrive.
  // Used to determine BX boundaries and TOA behavior.
  double measTime = getCondition(channelID, "MEAS_TIME");
  double drainRate = getCondition(channelID, "DRAIN_RATE");
  double readoutThresholdFloat = this->readoutThreshold(channelID);
  int readoutThreshold = int(readoutThresholdFloat);

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
  bool wasTOA = false;
  for (int iADC = 0; iADC < nADCs_; iADC++) {
    double startBX = (iADC - i_soi_) * clock_cycle_ - measTime;
    ldmx_log(trace) << "  iADC = " << iADC << " at startBX = " << startBX;

    // step 3b: check each merged hit to see if it peaks in this BX.  If so,
    // check its peak time to see if it's over TOT or TOA.
    bool startTOT = false;
    bool overTOA = false;
    double toverTOA = -1;
    double toverTOT = -1;
    for (auto hit : pulse.hits()) {
      int hitBX = int((hit.second + measTime) / clock_cycle_ + i_soi_);
      // if this hit wasn't in the current BX, continue...
      if (hitBX != iADC) {
        continue;
      }

      double vpeak = pulse(hit.second);

      if (vpeak > totThreshold) {
        startTOT = true;
        // use the latest time in the window
        if (toverTOT < hit.second) {
          toverTOT = hit.second;
        }
      }

      if (vpeak > toaThreshold) {
        if (!overTOA || hit.second < toverTOA) toverTOA = hit.second;
        overTOA = true;
      }

    }  // loop over sim hits_

    // check for the case of a TOA even though the peak is in the next BX
    if (!overTOA && pulse(startBX + clock_cycle_) > toaThreshold) {
      if (pulse(startBX) < toaThreshold) {
        // pulse crossed TOA threshold somewhere between the start of this
        // basket and the end
        overTOA = true;
        toverTOA = startBX + clock_cycle_;
      }
    }

    if (startTOT) {
      // above TOT threshold -> do TOT readout mode

      // @TODO NO NOISE
      //  CompositePulse includes pedestal, we need to remove it
      //  when calculating the charge deposited.
      double charge_deposited =
          (pulse(toverTOT) - gain * pedestal) * padCapacitance;

      // Measure Time Over Threshold (TOT) by using the drain rate.
      //  1. Use drain rate to see how long it takes for the charge to drain off
      //  2. Translate this into DIGI samples

      // Assume linear drain with slope drain rate:
      //      y_-intercept = pulse amplitude
      //      slope       = drain rate
      //  ==> x-intercept = amplitude / rate
      // actual time over threshold using the real signal voltage amplitude
      double tot = charge_deposited / drainRate;
      ldmx_log(trace) << "    we are in TOT read-out mode, TOT = " << tot;

      // calculate the TDC counts for this tot measurement
      //  internally, the chip uses 12 bits (2^12 = 4096)
      //  to measure a maximum of tot Max [ns]
      int tdc_counts = int(tot * 4096 / totMax) + pedestal;

      // were we already over TOA?  TOT is reported in BX where TOA went over
      // threshold...
      int toa{0};
      if (wasTOA) {
        // TOA was in the past
        toa = digiToAdd.back().toa();
      } else {
        // TOA is here and we need to find it
        double timecross = pulse.findCrossing(startBX, toverTOT, toaThreshold);
        toa = int((timecross - startBX) * ns_);
        // keep inside valid limits
        if (toa == 0) toa = 1;
        if (toa > 1023) toa = 1023;
      }
      ldmx_log(trace) << "    Adding TOT hit with toa = " << toa
                      << ", tdc_counts = " << tdc_counts
                      << " adc_t at prev iADC = "
                      << digiToAdd.at(iADC - 1).adc_t();
      // ADC at t-1
      auto adc_at_tminus1 =
          (iADC > 0) ? digiToAdd.at(iADC - 1).adc_t() : pedestal;
      auto i_tot_sample = digiToAdd.size();
      // mark as a TOT measurement with 2nd boolean as true
      digiToAdd.emplace_back(false, true, adc_at_tminus1, tdc_counts, toa);

      // TODO: properly handle saturation and recovery, eventually.
      // Now just kill everything...
      ldmx_log(trace)
          << "   Adding further hits_ with ADC [t-1] = 0x3FF, toa = "
             "0x3FF, until digiToAdd.size() = "
          << digiToAdd.size() << " < nADCs_(" << nADCs_ << ")";
      while (digiToAdd.size() < nADCs_) {
        // flags to mark type of sample
        digiToAdd.emplace_back(true, false, 0x3FF, 0x3FF, 0);
      }
      // Read out if the toa is within one Bx after nominal
      return (i_tot_sample <= i_soi_ + 1);
    } else {
      // determine the voltage at the sampling time
      double bxvolts = pulse((iADC - i_soi_) * clock_cycle_);
      // add noise if requested
      if (noise_) bxvolts += noise(channelID);
      // convert to integer and keep in range (handle low and high saturation)
      int adc = bxvolts / gain;
      ldmx_log(trace) << "    we are in ADC read-out mode, adc = " << adc;
      if (adc < 0) adc = 0;
      if (adc > 1023) adc = 1023;

      // check for TOA
      int toa(0);
      if (pulse(startBX) < toaThreshold && overTOA) {
        double timecross = pulse.findCrossing(startBX, toverTOA, toaThreshold);
        toa = int((timecross - startBX) * ns_);
        // keep inside valid limits
        if (toa == 0) toa = 1;
        if (toa > 1023) toa = 1023;
        wasTOA = true;
      } else {
        wasTOA = false;
      }
      // ADC at t-1
      auto adc_t_minus1 =
          (iADC > 0) ? digiToAdd.at(iADC - 1).adc_t() : pedestal;

      digiToAdd.emplace_back(false, false, adc_t_minus1, adc, toa);
    }  // TOT or ADC Mode
  }  // sampling baskets

  if (savePulseTruthInfo_)
    pulseTruthColl_->push_back(ldmx::HgcrocPulseTruth(channelID, pulse));

  // we only get here if we never went into TOT mode
  // check the SOI to see if we should read out
  ldmx_log(trace) << "  we are adding the hit IFF iSOI= " << i_soi_
                  << "'s adc_t = " << digiToAdd.at(i_soi_).adc_t()
                  << " >= thresh (" << readoutThreshold << ")";
  return digiToAdd.at(i_soi_).adc_t() >= readoutThreshold;
}  // HgcrocEmulator::digitize

std::vector<ldmx::HgcrocDigiCollection::Sample> HgcrocEmulator::noiseDigi(
    const int &channel, const double &soi_amplitude) const {
  // get chip conditions from emulator
  double pedestal{this->pedestal(channel)};
  double gain{this->gain(channel)};
  // fill a digi with noise samples
  std::vector<ldmx::HgcrocDigiCollection::Sample> noise_digi;
  for (int iADC{0}; iADC < nADCs_; iADC++) {
    // gen noise for ADC samples
    // ADC at t-1
    int adc_tm1{static_cast<int>(pedestal)};
    if (iADC > 0) {
      adc_tm1 = noise_digi.at(iADC - 1).adc_t();
    } else {
      adc_tm1 += noise(channel) / gain;
    }
    // ADC at t
    int adc_t{static_cast<int>(pedestal + noise(channel) / gain)};

    if (iADC == i_soi_) adc_t += soi_amplitude / gain;

    // set toa to 0 (not determined)
    // put new sample into noise digi
    noise_digi.emplace_back(false, false, adc_tm1, adc_t, 0);
  }  // samples in noise digi
  return noise_digi;
}

}  // namespace ldmx
