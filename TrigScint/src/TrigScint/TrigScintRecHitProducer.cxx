#include "TrigScint/TrigScintRecHitProducer.h"

#include <iostream>

#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"

namespace trigscint {

TrigScintRecHitProducer::TrigScintRecHitProducer(const std::string& name,
                                                 framework::Process& process)
    : Producer(name, process) {}

TrigScintRecHitProducer::~TrigScintRecHitProducer() {}

void TrigScintRecHitProducer::configure(
    framework::config::Parameters& parameters) {
  // Configure this instance of the producer
  pedestal_ = parameters.get<double>("pedestal");
  gain_ = parameters.get<double>("gain");
  mev_per_mip_ = parameters.get<double>("mev_per_mip");
  pe_per_mip_ = parameters.get<double>("pe_per_mip");
  input_collection_ = parameters.get<std::string>("input_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  output_collection_ = parameters.get<std::string>("output_collection");
  sample_of_interest_ = parameters.get<int>("sample_of_interest");
}

void TrigScintRecHitProducer::produce(framework::Event& event) {
  // Initialize QIE object for linearizing ADCs
  SimQIE qie;

  // Retrieve the collection of QIE digis
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      input_collection_, input_pass_name_)};

  std::vector<ldmx::TrigScintHit> trig_scint_hits;

  // Loop over digis and process each one
  for (const auto& digi : digis) {
    ldmx::TrigScintHit hit;
    auto adc{digi.getADC()};
    auto tdc{digi.getTDC()};

    hit.setModuleID(0);
    hit.setBarID(digi.getChanID());
    hit.setBeamEfrac(-1.);

    // Set amplitude as the sum of the first two samples
    hit.setAmplitude(
        qie.adc2Q(adc[sample_of_interest_]) +
        qie.adc2Q(adc[sample_of_interest_ + 1]));  // femptocoulombs

    // Set time based on TDC value
    if (tdc[sample_of_interest_] > 49)
      hit.setTime(-999.);
    else
      hit.setTime(tdc[sample_of_interest_] * 0.5);

    float integrated_charge = 0;

    // Integrate pulse over all time samples and subtract pedestal
    for (const auto& adc_val : adc) {
      integrated_charge += qie.adc2Q(adc_val);
    }
    uint n_samp = adc.size();
    float ped_subtr_q = integrated_charge - n_samp * pedestal_;

    // Set energy and photoelectrons
    hit.setEnergy(ped_subtr_q * 6250. / gain_ * mev_per_mip_ /
                  pe_per_mip_);  // MeV
    hit.setPE(ped_subtr_q * 6250. / gain_);

    trig_scint_hits.push_back(hit);
  }

  // Add the processed hits to the event
  event.add(output_collection_, trig_scint_hits);
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintRecHitProducer);
