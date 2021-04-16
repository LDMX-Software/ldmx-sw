
#include "Tools/HgcrocEmulator.h"

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
  digiToAdd.clear();  // make sure it is clean

  // sum all voltages and do a voltage-weighted average to get the hit time
  //  exclude any hits with times outside the sampling region
  double signalAmplitude = 0.0;
  double timeInWindow = 0.0;
  for (int iContrib = 0; iContrib < voltages.size(); iContrib++) {
    if (times.at(iContrib) < 0 or times.at(iContrib) > clockCycle_ * nADCs_) {
      // invalid contribution - outside time range or time is unset
      continue;
    }

    signalAmplitude += voltages.at(iContrib);
    timeInWindow += voltages.at(iContrib) * times.at(iContrib);
  }
  if (signalAmplitude > 0.)
    timeInWindow /= signalAmplitude;  // voltage weighted average

  // put noise onto timing
  // TODO more physical way of simulating the timing jitter
  if (noise_) timeInWindow += noiseInjector_->Gaus(0., timingJitter_);

  // set time in the window to zero if noise pushed it below zero
  // TODO better (more physical) method for handling this case?
  if (timeInWindow < 0.) timeInWindow = 0.;

  // setup up pulse by changing the amplitude and timing parameters
  configurePulse(signalAmplitude, timeInWindow);

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
            << "readout: " << readoutThreshold << " mV, "
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

  // choose readout mode
  double pulsePeak = measurePulse(timeInWindow, false);
  if (verbose_) {
    std::cout << "Pulse: { "
              << "Amplitude: " << pulsePeak << "mV, "
              << "Beginning: " << measurePulse(0., false) << "mV, "
              << "Time: " << timeInWindow << "ns } -> ";
  }
  if (pulsePeak < totThreshold) {
    /**
     * TODO more realistic ADC readout
     *
     * A real ADC readout would sum the pulses at the different
     * sampling times (instead of sampling one pulse after adding
     * together the amplitudes). This would involve several additional
     * complexities that aren't currently integrated.
     *  - Does the hit time directly correspond to the peak time? (as is now)
     *    Or should we shift the peak time to some time after the hit time?
     *  - How does the pre-amp shape several analog pulses coming together
     *    when they are separated by time? Do they just add linearly?
     */

    // below TOT threshold -> do ADC readout mode
    if (verbose_) std::cout << "ADC Mode { ";

    // measure time of arrival (TOA) using TOA threshold
    double toa(0.);
    // make sure pulse crosses TOA threshold
    if (measurePulse(0., false) < toaThreshold and pulsePeak > toaThreshold) {
      toa = pulseFunc_.GetX(toaThreshold - gain * pedestal,
                            -nADCs_ * clockCycle_, timeInWindow);
    }
    if (verbose_) std::cout << "TOA: " << toa << "ns, ";

    // measure ADCs
    for (unsigned int iADC = 0; iADC < nADCs_; iADC++) {
      double fullMeasTime = iADC * clockCycle_ + measTime;
      digiToAdd.emplace_back(
          false, false,  // use flags to mark this sample as an ADC measurement
          iADC > 0 ? digiToAdd.at(iADC - 1).adc_t()
                   : pedestal,  // ADC t-1 is first measurement
          measurePulse(fullMeasTime, noise_) /
              gain,  // ADC t is second measurement
          toa * ns_  // TOA is third measurement
      );
      if (verbose_)
        std::cout << " ADC " << iADC << ": " << digiToAdd[iADC].adc_t() << ", ";
    }
    if (verbose_) std::cout << "}" << std::endl;

    /**
     * Determine whether to readout this hit depending on the readout
     * threshold
     */
    return (digiToAdd.at(iSOI_).adc_t() >= readoutThreshold);

  } else {
    /**
     * TODO more realistic TOT readout
     *
     * A real TOT readout would invalidate the cell for any samples after the
     * hit started until the ADC(t-1) sample is able to recover. The TOT readout
     * is always given in the SOI for the hit that is being TOT readout (TOT
     * complete), but any bunches after that hit where the chip hasn't recovered
     * yet would recieve TOT in progress.
     */
    // above TOT threshold -> do TOT readout mode

    double charge_deposited = signalAmplitude * readoutPadCapacitance_;

    // Measure Time Over Threshold (TOT) by using the drain rate.
    //      1. Use drain rate to see how long it takes for the charge to drain
    //      off
    //      2. Translate this into DIGI samples

    // Assume linear drain with slope drain rate:
    //      y-intercept = pulse amplitude
    //      slope       = drain rate
    //  ==> x-intercept = amplitude / rate
    // actual time over threshold using the real signal voltage amplitude
    double tot = charge_deposited / drainRate;

    double toa(0.);  // default is earliest possible time
    // check if first half is just always above readout
    if (measurePulse(0., false) < totThreshold)
      toa = pulseFunc_.GetX(totThreshold - gain * pedestal, 0., timeInWindow);

    // calculate the index that tot will complete on
    int num_whole_clocks = int(tot / clockCycle_);

    // calculate the TDC counts for this tot measurement
    //  internally, the chip uses 12 bits (2^12 = 4096)
    //  to measure a maximum of tot Max [ns]
    int tdc_counts = int(tot * 4096 / totMax_) + pedestal;

    if (verbose_) {
      std::cout << "TOT Mode { "
                << "TOA: " << toa << " ns, "
                << "TOT: " << tot << " ns, "
                << "TDC: " << tdc_counts << "}" << std::endl;
    }

    // Notice that if tot > max time = digiToAdd.size()*clockCycle_,
    //  this will return just the max time
    for (unsigned int iADC = 0; iADC < nADCs_; iADC++) {
      bool tot_progress, tot_complete;
      int secon_measurement;
      if (iADC == iSOI_) {
        // for in-time hits, the TOT is reported in the SOI
        secon_measurement = tdc_counts;
        tot_progress = false;
        tot_complete = true;
      } else {
        // TOT still in progress or already completed
        double fullMeasTime = iADC * clockCycle_ + measTime;
        // TODO what does the pulse ADC measurement look like during TOT
        secon_measurement = measurePulse(fullMeasTime, noise_) / gain;
        tot_progress = (iADC < num_whole_clocks);
        tot_complete = false;
      }
      digiToAdd.emplace_back(
          tot_progress, tot_complete,  // flags to mark type of sample
          iADC > 0 ? digiToAdd.at(iADC - 1).adc_t()
                   : pedestal,  // first measurement is ADC t-1
          secon_measurement,
          toa * ns_  // last measurement is TOA
      );
    }

    /**
     * Always readout TOT hits
     */
    return true;
  }  // where is the amplitude of the hit w.r.t. readout and TOT thresholds

}  // HgcrocEmulator::digitize

}  // namespace ldmx
