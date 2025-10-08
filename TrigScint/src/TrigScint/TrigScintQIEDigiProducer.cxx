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
  strips_per_array_ = parameters.get<int>("number_of_strips");
  // number_of_arrays_ = parameters.get<int>("number_of_arrays");
  mean_noise_ = parameters.get<double>("mean_noise");
  mev_per_mip_ = parameters.get<double>("mev_per_mip");
  pe_per_mip_ = parameters.get<double>("pe_per_mip");
  input_collection_ = parameters.get<std::string>("input_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  output_collection_ = parameters.get<std::string>("output_collection");

  // QIE specific parameters initialization
  maxts_ = parameters.get<int>("maxts");
  toff_overall_ = parameters.get<double>("toff_overall");
  input_pulse_shape_ = parameters.get<std::string>("input_pulse_shape");
  tdc_thr_ = parameters.get<double>("tdc_thr");
  pedestal_ = parameters.get<double>("pedestal");
  elec_noise_ = parameters.get<double>("elec_noise");
  sipm_gain_ = parameters.get<double>("sipm_gain");
  s_freq_ = parameters.get<double>("qie_sf");
  zero_supp_cut_ = parameters.get<double>("zeroSupp_in_pe");

  if (input_pulse_shape_ == "Expo") {
    pulse_params_.clear();
    pulse_params_.push_back(parameters.get<double>("expo_k"));
    pulse_params_.push_back(parameters.get<double>("expo_tmax"));

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
  ldmx_log(debug) << "zeroSupp_in_pe =" << zero_supp_cut_;
  ldmx_log(debug) << "pe_per_mip =" << pe_per_mip_;
  ldmx_log(debug) << "mev_per_mip =" << mev_per_mip_;
}

void TrigScintQIEDigiProducer::produce(framework::Event& event) {
  // Need to handle seeding on the first event
  if (random_.get() == nullptr) {
    const auto& rseed = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    const auto& rseed2 = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);

    random_ = std::make_unique<TRandom3>(rseed.getSeed(output_collection_));

    // Initialize SimQIE instance with
    // pedestal, electronic noise and the random seed
    smq_ = new SimQIE(pedestal_, elec_noise_,
                      rseed2.getSeed(output_collection_ + "SimQIE"));

    smq_->setGain(sipm_gain_);
    smq_->setFreq(s_freq_);
    smq_->setNTimeSamples(maxts_);
    smq_->setTDCThreshold(tdc_thr_);
  }

  // To simulate multiple pulses coming at different times, SiPMS
  // Initialize with strips_per_array_ zeros
  std::vector<float> true_edep(strips_per_array_, 0.);

  // Initialize with strips_per_array_ nullptrs
  std::vector<Expo*> ex(strips_per_array_, nullptr);
  for (int i = 0; i < strips_per_array_; i++) {
    // Set the pulse shape with fixed parameters given by config. file
    ex[i] = new Expo(pulse_params_[0], pulse_params_[1]);
    true_edep[i] = 0;
  }

  // loop over sim hits and aggregate energy depositions for each detID
  const auto sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      input_collection_, input_pass_name_)};

  for (const auto& sim_hit : sim_hits) {
    ldmx::TrigScintID id(sim_hit.getID());

    ldmx_log(debug) << "Processing sim hit with bar ID: " << id.bar();

    // Simulating the noise corresponding to uncertainity in
    // detecting scintillating photons.
    // Poissonian distribution with mean = mean PEs generated
    double pulse_amp =
        random_->Poisson(sim_hit.getEdep() / mev_per_mip_ * pe_per_mip_);

    // Adding a pulse for every sim hit recorded.
    // time offset = global offset+simhit time
    ex[id.bar()]->addPulse(toff_overall_ + sim_hit.getTime(), pulse_amp);

    // incrementing true energy deposited in appropriate bar.
    true_edep[id.bar()] += sim_hit.getEdep();
  }

  // A container to hold the digitized trigger scintillator hits.
  std::vector<trigscint::TrigScintQIEDigis> q_digis;

  double total_noise = mean_noise_ * maxts_;

  // time period[ns] = 1000/sampling freq.[MHz]
  double sampling_time = 1000 / s_freq_;

  // Loop over all the bars available.
  for (int bar_id = 0; bar_id < strips_per_array_; bar_id++) {
    // Dark current simulation
    // e-hole pairs may be generated at random times in SiPM
    // due to thermal fluctuations.
    // Every e- thus generated, mimicks a Photo Electron.
    // Hence we will creat 1PE pulses for each electron generated.
    int n_noise_pulses = random_->Poisson(total_noise);
    for (int i = 0; i < n_noise_pulses; i++) {
      ex[bar_id]->addPulse(random_->Uniform(0, maxts_ * sampling_time), 1);
    }

    // Storing the "good" digis
    if (smq_->pulseCut(ex[bar_id], zero_supp_cut_)) {
      trigscint::TrigScintQIEDigis qie_info;

      qie_info.setChanID(bar_id);
      qie_info.setADC(smq_->outAdc(ex[bar_id]));
      qie_info.setTDC(smq_->outTdc(ex[bar_id]));
      qie_info.setCID(smq_->capId(ex[bar_id]));

      q_digis.push_back(qie_info);
    }
  }
  event.add(output_collection_, q_digis);
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintQIEDigiProducer);
