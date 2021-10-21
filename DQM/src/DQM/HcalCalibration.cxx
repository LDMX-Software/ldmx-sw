
#include "DQM/HcalCalibration.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "DetDescr/HcalDigiID.h"

namespace dqm {

void HcalCalibration::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
}

void HcalCalibration::onProcessStart() {
  getHistoDirectory();
  int nChan = 384;
  int nSample = 4;
  for (unsigned int channel{0}; channel < nChan; channel++) {
    histograms_.create("adc chan "+std::to_string(channel),
      "Sample", nSample, 0, nSample, "ADC", 200, 0, 200);
  }
}

void HcalCalibration::analyze(const framework::Event& event) {
  // need to tell collection to decode in v2 style
  // TODO make stored in ROOT file
  ldmx::HgcrocDigiCollection::setVersion(2);

  auto digis{event.getObject<ldmx::HgcrocDigiCollection>(input_name_,input_pass_)};
  /**
   * we can do this incrementing of channel indices because the decoder
   * uses a map which sorts by electronic ID and then adds the digis in
   * sequence, so the order from event to event is the same esp without
   * zero supp
   */
  unsigned int i_digi{1};
  for (auto const& digi : digis) {
    unsigned int i_sample{0};
    for (auto const& sample : digi) {
      histograms_.fill("adc chan "+std::to_string(i_digi-1), i_sample, sample.adc_t());
      i_sample++;
    }
    i_digi++;
  }

}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, HcalCalibration)
