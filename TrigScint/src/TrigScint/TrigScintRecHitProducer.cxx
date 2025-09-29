#include "TrigScint/TrigScintRecHitProducer.h"

#include <iostream>

#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"

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
  sample_of_interest_ = parameters.getParameter<int>("sample_of_interest");
  
  // New options for handling pileup (hits with TDC==62)
  pe_cut_= parameters.getParameter<double>("pe_cut");
  reject_62tdc_ = parameters.getParameter<bool>("Reject_62TDC");
  recover_62tdc_ = parameters.getParameter<bool>("Recover_62TDC");
  // Fail if both are enabled
  if (reject_62tdc_ && recover_62tdc_){
    EXCEPTION_RAISE("Configuration",
                    "Invalid configuration: Reject_62TDC and Recover_62TDC cannot both be enabled.");
  }
}

void TrigScintRecHitProducer::produce(framework::Event &event) {
  // initialize QIE object for linearizing ADCs
  SimQIE qie;

  // Ensure the sample of interest <4
  /* // this assumes we are in well-behaved simulation land, not test beam
  wilderness if(sample_of_interest_>3) { ldmx_log(error)<<"sample_of_interest_
  should be one of 0,1,2,3\n"
                   <<"Currently, sample_of_interest = "<<sample_of_interest_
                   <<"\n";
    return;
  }
  */

  // looper over sim hits and aggregate energy depositions
  // for each detID
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      inputCollection_, inputPassName_)};

  std::vector<ldmx::TrigScintHit> trigScintHits;
  for (const auto &digi : digis) {
    ldmx::TrigScintHit hit;
    auto adc{digi.getADC()};
    auto tdc{digi.getTDC()};

    hit.setModuleID(0);
    hit.setBarID(digi.getChanID());
    hit.setBeamEfrac(-1.);

    // leave amplitude as sum of the first two
    hit.setAmplitude(
        qie.ADC2Q(adc[sample_of_interest_]) +
        qie.ADC2Q(adc[sample_of_interest_ + 1]));  // femptocoulombs

    if (tdc[sample_of_interest_] > 49)
      hit.setTime(-999.);
    else
      hit.setTime(tdc[sample_of_interest_] * 0.5);

    float integratedCharge = 0;
    // integrate pulse over all time samples. will subtract pedestal next
    for (const auto &adcVal : adc) {
      integratedCharge += qie.ADC2Q(adcVal);
    }
    uint nSamp = adc.size();
    float pedSubtrQ = integratedCharge - nSamp * pedestal_;
    hit.setEnergy(pedSubtrQ * 6250. / gain_ * mevPerMip_ / pePerMip_);  // MeV
    float pe = (pedSubtrQ * 6250. / gain_);
    hit.setPE(pe);

    // Apply user-selected pileup handling (default: accept all)
    // Compute PEs at the primary time sample only (in-time bunch)
    float ChargeOfInterest = qie.ADC2Q(adc[sample_of_interest_]);
    float pedQ = ChargeOfInterest - pedestal_;
    float pe_primary = (pedQ * 6250./gain_);
 
    if (reject_62tdc_ && tdc[sample_of_interest_] == 62) {
      continue;  // Reject all 62 TDC hits
    }
    if (recover_62tdc_ && tdc[sample_of_interest_] == 62 &&
        pe_primary < pe_cut_) {
      continue;  // Reject only low PEs (pileup hits) 
    }

    trigScintHits.push_back(hit);
  }
  // Create the container to hold the
  // digitized trigger scintillator hits.

  event.add(outputCollection_, trigScintHits);
}
}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintRecHitProducer);
