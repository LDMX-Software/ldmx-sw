#include "TrigScint/TrigScintRecHitProducer.h"
#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"

#include <iostream>

namespace trigscint {

TrigScintRecHitProducer::TrigScintRecHitProducer(const std::string &name,
                                                 framework::Process &process)
    : Producer(name, process) {}

TrigScintRecHitProducer::~TrigScintRecHitProducer() {}

void TrigScintRecHitProducer::configure(
    framework::config::Parameters &parameters) {
  // Configure this instance of the producer
  pedestal_ = parameters.getParameter<double>("pedestal");
  gain_ = parameters.getParameter<double>("gain");
  mevPerMip_ = parameters.getParameter<double>("mev_per_mip");
  pePerMip_ = parameters.getParameter<double>("pe_per_mip");
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  verbose_ = parameters.getParameter<bool>("verbose");
}

void TrigScintRecHitProducer::produce(framework::Event &event) {
  // initialize QIE object for linearizing ADCs
  SimQIE qie;

  // looper over sim hits and aggregate energy depositions
  // for each detID
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      inputCollection_, inputPassName_)};

  std::vector<ldmx::TrigScintHit> trigScintHits;
  for (const auto &digi : digis) {
    ldmx::TrigScintHit hit;
    auto adc = (std::vector<int>)digi.GetADC();
    auto tdc = (std::vector<int>)digi.GetTDC();

    hit.setModuleID(0);
    hit.setBarID(digi.GetChanID());
    hit.setBeamEfrac(-1.);

    hit.setAmplitude(qie.ADC2Q(digi.adcs_[1]) +
                     qie.ADC2Q(digi.adcs_[2]));  // femptocoulombs

    if (digi.tdcs_[1] > 49)
      hit.setTime(-999.);
    else
      hit.setTime(tdc[1] * 0.5);

    hit.setEnergy((qie.ADC2Q(adc[1]) + qie.ADC2Q(adc[2]) - pedestal_) * 6250. /
                  gain_ * mevPerMip_ / pePerMip_);  // MeV
    hit.setPE((qie.ADC2Q(adc[1]) + qie.ADC2Q(adc[2]) - pedestal_) * 6250. /
              gain_);
    trigScintHits.push_back(hit);
  }
  // Create the container to hold the
  // digitized trigger scintillator hits.

  event.add(outputCollection_, trigScintHits);
}
}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TrigScintRecHitProducer);
