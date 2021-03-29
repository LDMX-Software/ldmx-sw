
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

  hit_merge_ns_ = 0.05; // combine at 50 ps level
  
  // Configure the pulse shape function
  pulseFunc_ =
      TF1("pulseFunc",
          "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/"
          "((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
          0.0, (double)nADCs_ * clockCycle_);
  pulseFunc_.FixParameter(0, 1.0); // amplitude is set externally
  pulseFunc_.FixParameter(1, rateUpSlope_);
  pulseFunc_.FixParameter(2, timeUpSlope_);
  pulseFunc_.FixParameter(3, timePeak_);
  pulseFunc_.FixParameter(4, 0); // not using time offset in this way
  pulseFunc_.FixParameter(5, rateDnSlope_);
  pulseFunc_.FixParameter(6, timeDnSlope_);
}

void HgcrocEmulator::seedGenerator(uint64_t seed) {
  noiseInjector_ = std::make_unique<TRandom3>(seed);
}

class CompositePulse {
 public:
  typedef std::pair<double,double> ETPair;
  
  CompositePulse(TF1& func) : pulseFunc_{func} { }

  void addOrMerge(const ETPair& hit, double hit_merge_ns) {
    std::vector<ETPair>::iterator imerge;
    for (imerge = energy_time_.begin(); imerge!=energy_time_.end(); imerge++) 
      if (fabs(imerge->second-hit.second)<hit_merge_ns) break;
    if (imerge == energy_time_.end()) { // didn't find a match, add to the list
      energy_time_.push_back(hit);
    } else { // merge hits, shifting time to average
      imerge->second=(imerge->second*imerge->first+hit.first*hit.second);
      imerge->first+=hit.first;
      imerge->second/=imerge->first;
    }
  }

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

  void setGainPedestal(double gain, double pedestal) {
    gain_=gain;
    pedestal_=pedestal;
  }

  double operator()(double time) const {
    return at(time);
  }

  double at(double time) const {
    double signal = gain_ * pedestal_;
    for (auto hit : energy_time_)
      signal += hit.first * pulseFunc_.Eval(time-hit.second);
    return signal;
  };

  const std::vector< ETPair >& hits() const { return energy_time_; }
  
 private:
  std::vector< ETPair > energy_time_;
  double gain_, pedestal_;
  TF1& pulseFunc_;
  
};

bool HgcrocEmulator::digitize(
    const int &channelID, const std::vector<double> &voltages,
    const std::vector<double> &times,
    std::vector<ldmx::HgcrocDigiCollection::Sample> &digiToAdd) const {
  digiToAdd.clear();  // make sure it is clean

  std::vector<std::pair<double, double> > voltage_time, merged_voltage_time;
  // step 0: sort by subhit amplitude
  for (size_t i=0; i< voltages.size(); i++)
    voltage_time.push_back(std::pair<double,double>(voltages[i],times[i]));

  std::sort(voltage_time.begin(), voltage_time.end(),
            [](const std::pair<double,double>& a, const std::pair<double,double>&b) {
              return a.first>b.first;
            }
            );
  
  // step 1: gather voltages into groups separated by (programmable) ns, single pass
  CompositePulse pulse(pulseFunc_);

  for (auto hit : voltage_time)
    pulse.addOrMerge(hit, hit_merge_ns_);  

  // TODO step 2: add timing jitter

  bool doReadout=false;
  
  // Configure chip settings based off of table (that may have been passed)
  double gain = getCondition(channelID, "gain", gain_);
  double pedestal = getCondition(channelID, "pedestal", pedestal_);
  double toaThreshold = getCondition(channelID, "toaThreshold", toaThreshold_);
  double totThreshold = getCondition(channelID, "totThreshold", totThreshold_);
  // measTime defines the point in the BX where an in-time (time=0 in times vector) hit would arrive.  Used to determine BX boundaries and TOA behavior.
  double measTime = getCondition(channelID, "measTime", measTime_); 
  double drainRate = getCondition(channelID, "drainRate", drainRate_);

  pulse.setGainPedestal(gain, pedestal);
  
  double readoutThresholdFloat =
      getCondition(channelID, "readoutThreshold", readoutThreshold_);
  int readoutThreshold = int(readoutThresholdFloat);
  /* debug printout */
  /*
  std::cout << "Configuration: {"
            << "gain: " << gain << " mV/ADC, "
            << "pedestal: " << pedestal << ", "
            << "readout: " << readoutThreshold << " mV, "
            << "toa: " << toaThreshold << " mV, "
            << "tot: " << totThreshold << " mV, "
            << "measTime: " << measTime << " ns, "
            << "iSOI: " << iSOI_ << "bx, "
            << "clockCycle: " << clockCycle_ << "ns, "
            << "drainRate: " << drainRate << " mV/ns }"
            << std::endl;
  */

  /// the time here is nominal (zero gives peak if hit.second is zero)
  
  // step 3: go through each BX sample one by one
  bool wasTOA=false;  
  for (int iADC = 0; iADC < nADCs_; iADC++) {
   
    double startBX=(iADC-iSOI_)*clockCycle_ - measTime_;
    //    std::cout << iADC << " " <<startBX << std::endl;
    
    // step 3b: check each merged hit to see if it peaks in this BX.  If so, check its peak time to see if it's over TOT or TOA.
    bool startTOT=false;
    bool overTOA=false;
    double toverTOA=-1;
    double toverTOT=-1;
    for (auto hit: pulse.hits()) {
      int hitBX=int((hit.second+measTime_)/clockCycle_+iSOI_);
      if (hitBX!=iADC) continue; // if this hit wasn't in the current BX, continue...
      double vpeak=pulse(hit.second);
      
      if (vpeak>totThreshold) {
        startTOT=true;
        if (toverTOT<hit.second) toverTOT=hit.second; // use the latest time in the window
      }

      //      std::cout << "I peak here! " << hit.second << "/" << hit.second+measTime_ << " " << iADC << " " << hit.first << " " << vpeak << " ";

      if (vpeak>toaThreshold) {
        if (!overTOA || hit.second<toverTOA) toverTOA=hit.second;        
        overTOA=true;
        //        std::cout << "TOA";
      }
      
      //      std::cout << std::endl;
    }
    // check for the case of a TOA even though the peak is in the next BX
    if (!overTOA && pulse(startBX+clockCycle_)>toaThreshold) {
      if (pulse(startBX)<toaThreshold) { // need to be under threshold to start with for a TOA

        //        std::cout << "TOA pre-peak " << iADC << std::endl;
        overTOA=true;
        toverTOA=startBX+clockCycle_;
      }
    }
    
    if (startTOT) {

      // above TOT threshold -> do TOT readout mode

      double charge_deposited = pulse(toverTOT) * readoutPadCapacitance_;

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

      // calculate the index that tot will complete on
      int num_whole_clocks = int(tot / clockCycle_);

      // calculate the TDC counts for this tot measurement
      //  internally, the chip uses 12 bits (2^12 = 4096)
      //  to measure a maximum of tot Max [ns]
      int tdc_counts = int(tot * 4096 / totMax_) + pedestal;
      
      // were we already over TOA?  TOT is reported in BX where TOA went over threshold...
      if (wasTOA) {
        //        std::cout << "TOT is in the past..." << std::endl;
        digiToAdd.back()=HgcrocDigiCollection::Sample(false,true,
                                                       digiToAdd.back().adc_tm1(),
                                                       tdc_counts,
                                                       digiToAdd.back().toa());
      } else { // need to find the TOA...
        //        std::cout << "TOT/TOA is here..." << std::endl;
        double timecross=pulse.findCrossing(startBX,toverTOT,toaThreshold);
        int toa=int((timecross-startBX)*ns_);
        std::cout << timecross << " " <<toa << std::endl;
        // keep inside valid limits
        if (toa==0) toa=1;
        if (toa>1023) toa=1023;

        digiToAdd.emplace_back(
            false, true,  // mark as a TOT measurement
            (iADC > 0) ? digiToAdd.at(iADC - 1).adc_t() : pedestal, // ADC t-1 is first measurement
            tdc_counts, // TOT
            toa // TOA is third measurement
                               );
        
        
      }

      // TODO: properly handle saturation and recovery, eventually.  Now just kill everything...
      while (digiToAdd.size()<nADCs_) {
        digiToAdd.emplace_back(
            true, false,  // flags to mark type of sample
            0x3FF,
            0x3FF,
            0);
      }

      return true; // always readout
    } else {
      // determine the voltage at the sampling time
      double bxvolts=pulse((iADC-iSOI_)*clockCycle_);
      // add noise if requested
      if (noise_) bxvolts+=noiseInjector_->Gaus(0., noiseRMS_);
      // convert to integer and keep in range (handle low and high saturation)
      int adc=bxvolts/gain;
      if (adc<0) adc=0;
      if (adc>1023) adc=1023;

      // check for TOA
      int toa(0);
      if (pulse(startBX)<toaThreshold && overTOA) {
        //        std::cout << "TOA is here...";
        double timecross=pulse.findCrossing(startBX,toverTOA,toaThreshold);
        toa=int((timecross-startBX)*ns_);
        //  std::cout << timecross << " " << timecross-startBX << " " <<toa << std::endl;
        // keep inside valid limits
        if (toa==0) toa=1;
        if (toa>1023) toa=1023;
        wasTOA=true;
      } else {
        wasTOA=false;
      }

      digiToAdd.emplace_back(
          false, false,  // use flags to mark this sample as an ADC measurement
          (iADC > 0) ? digiToAdd.at(iADC - 1).adc_t() : pedestal, // ADC t-1 is first measurement
          adc, // ADC[t] is the second field
          toa // TOA is third measurement
                             );
      
      if (iADC==iSOI_ && adc>=readoutThreshold_) doReadout=true;      
    }
  }
      return doReadout;
}  // HgcrocEmulator::digitize

}  // namespace ldmx
