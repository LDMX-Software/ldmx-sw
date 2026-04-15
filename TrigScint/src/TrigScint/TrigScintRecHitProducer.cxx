#include "TrigScint/TrigScintRecHitProducer.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"

namespace trigscint {

TrigScintRecHitProducer::TrigScintRecHitProducer(const std::string &name,
                                                 framework::Process &process)
    : Producer(name, process) {}

TrigScintRecHitProducer::~TrigScintRecHitProducer() {}

void TrigScintRecHitProducer::configure(framework::config::Parameters &parameters) {
  pedestal_ = parameters.get<double>("pedestal");
  gain_ = parameters.get<double>("gain");
  mev_per_mip_ = parameters.get<double>("mev_per_mip");
  pe_per_mip_ = parameters.get<double>("pe_per_mip");
  input_collection_ = parameters.get<std::string>("input_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  output_collection_ = parameters.get<std::string>("output_collection");
  sample_of_interest_ = parameters.get<int>("sample_of_interest");

  use_calib_file_ = parameters.get<bool>("use_calib_file", false);
  calib_file_ = parameters.get<std::string>("calib_file", std::string{""});

  // If > 0: integrate [sample_of_interest, sample_of_interest + integration_window)
  // If <= 0: integrate all samples (default)
  integration_window_ = parameters.get<int>("integration_window", 0);

  if (use_calib_file_) {
    if (calib_file_.empty()) {
      throw std::runtime_error(
          "TrigScintRecHitProducer: use_calib_file=true but calib_file is empty.");
    }
    readCalib(calib_file_, &gains_, &pedestals_);
  }
}

void TrigScintRecHitProducer::produce(framework::Event &event) {
  SimQIE qie;

  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      input_collection_, input_pass_name_)};

  std::vector<ldmx::TrigScintHit> trig_scint_hits;
  trig_scint_hits.reserve(digis.size());

  for (const auto &digi : digis) {
    ldmx::TrigScintHit hit;
    auto adc{digi.getADC()};
    auto tdc{digi.getTDC()};

    hit.setModuleID(0);
    hit.setBarID(digi.getChanID());
    hit.setBeamEfrac(-1.);

    const int soi = sample_of_interest_;
    if (soi < 0 || soi >= int(adc.size()) || soi >= int(tdc.size())) {
      continue;
    }
    if (soi + 1 < int(adc.size())) {
      hit.setAmplitude(qie.adc2Q(adc[soi]) + qie.adc2Q(adc[soi + 1]));
    } else {
      hit.setAmplitude(qie.adc2Q(adc[soi]));
    }
    if (tdc[soi] > 49)
      hit.setTime(-999.);
    else
      hit.setTime(tdc[soi] * 0.5);

    int start = soi;
    int end = int(adc.size());
    if (integration_window_ > 0) {
      end = std::min(start + integration_window_, int(adc.size()));
    }
    const int n_samp = std::max(0, end - start);

    float integrated_charge = 0.f;
    for (int i = start; i < end; ++i) {
      integrated_charge += qie.adc2Q(adc[i]);
    }

    double ped = pedestal_;
    double gain = gain_;

    if (use_calib_file_) {
      const int ch = digi.getChanID();
      if (ch >= 0 && ch < int(pedestals_.size())) ped = pedestals_[ch];
      if (ch >= 0 && ch < int(gains_.size())) gain = gains_[ch];
    }

    const float ped_subtr_q = integrated_charge - float(n_samp) * float(ped);

    hit.setEnergy(ped_subtr_q * 6250.f / float(gain) * float(mev_per_mip_) /
                  float(pe_per_mip_));  // MeV
    hit.setPE(ped_subtr_q * 6250.f / float(gain));

    trig_scint_hits.push_back(hit);
  }
  event.add(output_collection_, trig_scint_hits);
}

void TrigScintRecHitProducer::readCalib(const std::string &filename,
                                       std::vector<double> *gains,
                                       std::vector<double> *pedestals) {
  std::ifstream infile(filename);
  if (!infile) {
    throw std::runtime_error(
        "TrigScintRecHitProducer: Could not open calibration file: " + filename);
  }

  double ch = 0.;
  double gain = 0.;
  double ped = 0.;

  while (infile >> ch >> gain >> ped) {
    const int bar_id = int(ch);
    if (bar_id < 0) continue;
    if (bar_id >= int(gains->size())) {
      gains->resize(bar_id + 1, 0.0);
      pedestals->resize(bar_id + 1, 0.0);
    }
    (*gains)[bar_id] = gain;
    (*pedestals)[bar_id] = ped;
  }
}

}  

DECLARE_PRODUCER(trigscint::TrigScintRecHitProducer);