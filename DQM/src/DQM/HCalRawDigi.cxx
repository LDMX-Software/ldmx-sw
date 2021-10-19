
#include "DQM/HCalRawDigi.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "DetDescr/HcalElectronicsID.h"

namespace dqm {

void HCalRawDigi::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
}

void HCalRawDigi::onProcessStart() {
  getHistoDirectory();
  histograms_.create("adc_by_channel",
      "HCal EID Index", 300, 0, 300, "ADC Counts in SOI", 200, 0, 200);
}

void HCalRawDigi::analyze(const framework::Event& event) {
  // need to tell collection to decode in v2 style
  // TODO make configurable
  ldmx::HgcrocDigiCollection::setVersion(2);

  auto digis{event.getObject<ldmx::HgcrocDigiCollection>(input_name_,input_pass_)};
  for (auto const& digi : digis) {
    ldmx::HcalElectronicsID eid(digi.id());
    histograms_.fill("adc_by_channel",eid.index(),digi.soi().adc_t());
  }

}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalRawDigi)
