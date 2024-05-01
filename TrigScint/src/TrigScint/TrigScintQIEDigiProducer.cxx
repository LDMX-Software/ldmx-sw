#include "TrigScint/TrigScintQIEDigiProducer.h"

#include <iostream>

#include "Framework/Exception/Exception.h"
#include "Framework/Logger.h"
#include "Framework/RandomNumberSeedService.h"

namespace trigscint {

TrigScintQIEDigiProducer::TrigScintQIEDigiProducer(const std::string& name,
                                                   framework::Process& process)
    : Producer(name, process) {}

void TrigScintQIEDigiProducer::configure(
    framework::config::Parameters& parameters) {
  // Configure this instance of the producer
  stripsPerArray_ = parameters.getParameter<int>("number_of_strips");
  numberOfArrays_ = parameters.getParameter<int>("number_of_arrays");
  meanNoise_ = parameters.getParameter<double>("mean_noise");
  mevPerMip_ = parameters.getParameter<double>("mev_per_mip");
  pePerMip_ = parameters.getParameter<double>("pe_per_mip");
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  verbose_ = parameters.getParameter<bool>("verbose");

  // QIE specific parameters initialization
  maxts_ = parameters.getParameter<int>("maxts");
  toff_overall_ = parameters.getParameter<double>("toff_overall");
  input_pulse_shape_ =
      parameters.getParameter<std::string>("input_pulse_shape");
  tdc_thr_ = parameters.getParameter<double>("tdc_thr");
  pedestal_ = parameters.getParameter<double>("pedestal");
  elec_noise_ = parameters.getParameter<double>("elec_noise");
  sipm_gain_ = parameters.getParameter<double>("sipm_gain");
  s_freq_ = parameters.getParameter<double>("qie_sf");
  zeroSuppCut_ = parameters.getParameter<double>("zeroSupp_in_pe");

  if (input_pulse_shape_ == "Expo") {
    pulse_params_.clear();
    pulse_params_.push_back(parameters.getParameter<double>("expo_k"));
    pulse_params_.push_back(parameters.getParameter<double>("expo_tmax"));

    ldmx_log(debug) << "expo_k =" << pulse_params_[0];
    ldmx_log(debug) << "expo_tmax =" << pulse_params_[1];
  }

  // Debug mode: print parameter values.
  ldmx_log(debug) << "maxts_ =" << maxts_;
  ldmx_log(debug) << "toff_overall_ =" << toff_overall_;
  ldmx_log(debug) << "input_pulse_shape_ =" << input_pulse_shape_;
  ldmx_log(debug) << "tdc_thr =" << tdc_thr_;
  ldmx_log(debug) << "pedestal =" << pedestal_;
  ldmx_log(debug) << "elec_noise =" << elec_noise_;
  ldmx_log(debug) << "sipm_gain =" << sipm_gain_;
  ldmx_log(debug) << "qie_sf =" << s_freq_;
  ldmx_log(debug) << "zeroSupp_in_pe =" << zeroSuppCut_;
  ldmx_log(debug) << "pe_per_mip =" << pePerMip_;
  ldmx_log(debug) << "mev_per_mip =" << mevPerMip_;
}

void TrigScintQIEDigiProducer::produce(framework::Event& event) {
  // Need to handle seeding on the first event
  if (random_.get() == nullptr) {
    const auto& rseed = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    const auto& rseed2 = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);

    random_ = std::make_unique<TRandom3>(rseed.getSeed(outputCollection_));

    // Initialize SimQIE instance with
    // pedestal, electronic noise and the random seed
    smq_ = new SimQIE(pedestal_, elec_noise_,
                      rseed2.getSeed(outputCollection_ + "SimQIE"));

    smq_->setGain(sipm_gain_);
    smq_->setFreq(s_freq_);
    smq_->setNTimeSamples(maxts_);
    smq_->setTDCThreshold(tdc_thr_);
  }

  // To simulate multiple pulses coming at different times, SiPMS
  // Initialize with stripsPerArray_ zeros
  std::vector<float> TrueEdep(stripsPerArray_, 0.);

  // Initialize with stripsPerArray_ nullptrs
  std::vector<Expo*> ex(stripsPerArray_, nullptr);
  for (int i = 0; i < stripsPerArray_; i++) {
    // Set the pulse shape with fixed parameters given by config. file
    ex[i] = new Expo(pulse_params_[0], pulse_params_[1]);
    TrueEdep[i] = 0;
  }

  // loop over sim hits and aggregate energy depositions for each detID
  const auto simHits{event.getCollection<ldmx::SimCalorimeterHit>(
      inputCollection_, inputPassName_)};

  for (const auto& simHit : simHits) {
    ldmx::TrigScintID id(simHit.getID());

    ldmx_log(debug) << "Processing sim hit with bar ID: " << id.bar();

    // Simulating the noise corresponding to uncertainity in
    // detecting scintillating photons.
    // Poissonian distribution with mean = mean PEs generated
    double PulseAmp =
        random_->Poisson(simHit.getEdep() / mevPerMip_ * pePerMip_);

    // Adding a pulse for every sim hit recorded.
    // time offset = global offset+simhit time
    ex[id.bar()]->AddPulse(toff_overall_ + simHit.getTime(), PulseAmp);

    // incrementing true energy deposited in appropriate bar.
    TrueEdep[id.bar()] += simHit.getEdep();
  }

  // A container to hold the digitized trigger scintillator hits.
  std::vector<trigscint::TrigScintQIEDigis> QDigis;

  double TotalNoise = meanNoise_ * maxts_;

  // time period[ns] = 1000/sampling freq.[MHz]
  double SamplingTime = 1000 / s_freq_;

  // Loop over all the bars available.
  for (int bar_id = 0; bar_id < stripsPerArray_; bar_id++) {
    // Dark current simulation
    // e-hole pairs may be generated at random times in SiPM
    // due to thermal fluctuations.
    // Every e- thus generated, mimicks a Photo Electron.
    // Hence we will creat 1PE pulses for each electron generated.
    int n_noise_pulses = random_->Poisson(TotalNoise);
    for (int i = 0; i < n_noise_pulses; i++) {
      ex[bar_id]->AddPulse(random_->Uniform(0, maxts_ * SamplingTime), 1);
    }

    // Storing the "good" digis
    if (smq_->PulseCut(ex[bar_id], zeroSuppCut_)) {
      trigscint::TrigScintQIEDigis QIEInfo;

      QIEInfo.setChanID(bar_id);
      QIEInfo.setADC(smq_->Out_ADC(ex[bar_id]));
      QIEInfo.setTDC(smq_->Out_TDC(ex[bar_id]));
      QIEInfo.setCID(smq_->CapID(ex[bar_id]));

      QDigis.push_back(QIEInfo);
    }
  }
  event.add(outputCollection_, QDigis);
}

}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TrigScintQIEDigiProducer);
