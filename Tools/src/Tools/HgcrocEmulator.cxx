
#include "Tools/HgcrocEmulator.h"

namespace ldmx {

HgcrocEmulator::HgcrocEmulator(const framework::config::Parameters &ps) {
  // settings of readout chip that are the same for all chips
  //  used  in actual digitization
  noise_ = ps.getParameter<bool>("noise");
  noiseRMS_ = ps.getParameter<double>("noiseRMS");
  timingJitter_ = ps.getParameter<double>("timingJitter");
  rateUpSlope_ = ps.getParameter<double>("rateUpSlope");
  timeUpSlope_ = ps.getParameter<double>("timeUpSlope");
  rateDnSlope_ = ps.getParameter<double>("rateDnSlope");
  timeDnSlope_ = ps.getParameter<double>("timeDnSlope");
  timePeak_ = ps.getParameter<double>("timePeak");
  clockCycle_ = ps.getParameter<double>("clockCycle");
  nADCs_ = ps.getParameter<int>("nADCs");
  iSOI_ = ps.getParameter<int>("iSOI");

  // Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024. / clockCycle_;

  hit_merge_ns_ = 0.05;  // combine at 50 ps level

  // Configure the pulse shape function
  pulseFunc_ =
      TF1("pulseFunc",
          "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/"
          "((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
          0.0, (double)nADCs_ * clockCycle_);
  pulseFunc_.FixParameter(0, 1.0);  // amplitude is set externally
  pulseFunc_.FixParameter(1, rateUpSlope_);
  pulseFunc_.FixParameter(2, timeUpSlope_);
  pulseFunc_.FixParameter(3, timePeak_);
  pulseFunc_.FixParameter(4, 0);  // not using time offset in this way
  pulseFunc_.FixParameter(5, rateDnSlope_);
  pulseFunc_.FixParameter(6, timeDnSlope_);
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
  CompositePulse pulse(pulseFunc_, gain, pedestal);

  for (auto hit : arriving_pulses) pulse.addOrMerge(hit, hit_merge_ns_);

  // TODO step 2: add timing jitter
  // if (noise_) pulse.jitter();

  /// the time here is nominal (zero gives peak if hit.second is zero)

  // step 3: go through each BX sample one by one
  bool doReadout = false;
  bool wasTOA = false;
  for (int iADC = 0; iADC < nADCs_; iADC++) {
    double startBX = (iADC - iSOI_) * clockCycle_ - measTime;

    // step 3b: check each merged hit to see if it peaks in this BX.  If so,
    // check its peak time to see if it's over TOT or TOA.
    bool startTOT = false;
    bool overTOA = false;
    double toverTOA = -1;
    double toverTOT = -1;
    for (auto hit : pulse.hits()) {
      int hitBX = int((hit.second + measTime) / clockCycle_ + iSOI_);
      if (hitBX != iADC)
        continue;  // if this hit wasn't in the current BX, continue...

      double vpeak = pulse(hit.second);

      if (vpeak > totThreshold) {
        startTOT = true;
        if (toverTOT < hit.second)
          toverTOT = hit.second;  // use the latest time in the window
      }

      if (vpeak > toaThreshold) {
        if (!overTOA || hit.second < toverTOA) toverTOA = hit.second;
        overTOA = true;
      }

    }  // loop over sim hits

    // check for the case of a TOA even though the peak is in the next BX
    if (!overTOA && pulse(startBX + clockCycle_) > toaThreshold) {
      if (pulse(startBX) < toaThreshold) {
        // pulse crossed TOA threshold somewhere between the start of this
        // basket and the end
        overTOA = true;
        toverTOA = startBX + clockCycle_;
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
      //      y-intercept = pulse amplitude
      //      slope       = drain rate
      //  ==> x-intercept = amplitude / rate
      // actual time over threshold using the real signal voltage amplitude
      double tot = charge_deposited / drainRate;

      // calculate the index that tot will complete on
      int num_whole_clocks = int(tot / clockCycle_);

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

      digiToAdd.emplace_back(
          false, true,  // mark as a TOT measurement
          (iADC > 0) ? digiToAdd.at(iADC - 1).adc_t()
                     : pedestal,  // ADC t-1 is first measurement
          tdc_counts,             // TOT
          toa                     // TOA is third measurement
      );

      // TODO: properly handle saturation and recovery, eventually.  
      // Now just kill everything...
      while (digiToAdd.size() < nADCs_) {
        digiToAdd.emplace_back(true, false,  // flags to mark type of sample
                               0x3FF, 0x3FF, 0);
      }

      return true;  // always readout
    } else {
      // determine the voltage at the sampling time
      double bxvolts = pulse((iADC - iSOI_) * clockCycle_);
      // add noise if requested
      if (noise_) bxvolts += noiseInjector_->Gaus(0., noiseRMS_);
      // convert to integer and keep in range (handle low and high saturation)
      int adc = bxvolts / gain;
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

      digiToAdd.emplace_back(
          false, false,  // use flags to mark this sample as an ADC measurement
          (iADC > 0) ? digiToAdd.at(iADC - 1).adc_t()
                     : pedestal,  // ADC t-1 is first measurement
          adc,                    // ADC[t] is the second field
          toa                     // TOA is third measurement
      );
    }  // TOT or ADC Mode
  }    // sampling baskets

  // we only get here if we never went into TOT mode
  // check the SOI to see if we should read out
  return digiToAdd.at(iSOI_).adc_t() >= readoutThreshold;
}  // HgcrocEmulator::digitize

}  // namespace ldmx
