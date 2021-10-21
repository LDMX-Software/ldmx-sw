
#include "DQM/HCalRawDigi.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "DetDescr/HcalDigiID.h"

namespace dqm {

void HCalRawDigi::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
}

void HCalRawDigi::onProcessStart() {
  getHistoDirectory();
  int nlayers = 20;
  int nstrips = 4;
  for (uint32_t i_layer{0}; i_layer < nlayers; i_layer++) {
    std::string layer = std::to_string(i_layer);
    for (uint32_t i_end{0}; i_end < 2; i_end++) {
      std::string end = std::to_string(i_end);
      histograms_.create("adc_soi_by_strip_end"+end+"_layer"+layer,
			 "HCal Strip", nstrips, 0, nstrips, "ADC Counts in SOI", 200, 0, 200);
      histograms_.get("adc_soi_by_strip_end"+end+"_layer"+layer)->SetCanExtend(TH1::kAllAxes);
    }

  for (unsigned int i_sample{0}; i_sample < 4; i_sample++) {
    histograms_.create("adc_by_channel_sample"+std::to_string(i_sample),
      "Arbitrary Channel Index", 250, 0, 250, 
      "ADC Counts in Sample "+std::to_string(i_sample), 200, 0, 200);
    //histograms_.get("adc_by_channel_sample"+std::to_string(i_sample))->SetCanExtend(TH1::kAllAxes);
  }
}

void HCalRawDigi::analyze(const framework::Event& event) {
  auto digis{event.getObject<ldmx::HgcrocDigiCollection>(input_name_,input_pass_)};
  /**
   * we can do this incrementing of channel indices because the decoder
   * uses a map which sorts by electronic ID and then adds the digis in
   * sequence, so the order from event to event is the same without zero supp
   */
  unsigned int i_digi{1};
  std::cout << "nsamples " <<	digis.getNumSamplesPerDigi() <<	std::endl;
  for (auto const& digi : digis) {
    ldmx::HcalDigiID did(digi.id());
    std::string layer = std::to_string(did.layer());
    std::string end = std::to_string(did.end());
    histograms_.fill("adc_soi_by_strip_end"+end+"_layer"+layer,did.strip(),digi.soi().adc_t());

    for (unsigned int i_sample{0}; i_sample < digis.getNumSamplesPerDigi(); i_sample++) {
      histograms_.fill("adc_by_channel_sample"+std::to_string(i_sample),
          i_digi,digi.at(i_sample).adc_t());
    }
    i_digi++;
  }
}
  
}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalRawDigi)
