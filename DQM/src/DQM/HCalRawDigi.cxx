
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
  for (auto const& digi : digis) {
    for (unsigned int i_sample{0}; i_sample < digis.getNumSamplesPerDigi(); i_sample++) {
      histograms_.fill("adc_by_channel_sample"+std::to_string(i_sample),
          i_digi,digi.at(i_sample).adc_t());
    }
    i_digi++;
  }

}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalRawDigi)
