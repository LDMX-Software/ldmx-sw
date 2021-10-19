
#include "DQM/HCalPedestals.h"

#include "Recon/Event/HgcrocDigiCollection.h"

#include "DetDescr/HcalElectronicsID.h"

namespace dqm {

void HCalPedestals::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
}

void HCalPedestals::analyze(const framework::Event& event) {
  ldmx::HgcrocDigiCollection::setVersion(2);

  auto digis{event.getObject<ldmx::HgcrocDigiCollection>(input_name_,input_pass_)};
  int i_digi{0};
  for (auto const& digi : digis) {
    histograms_.fill("adc_by_channel", i_digi++, digi.soi().adc_t());
  }

}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalPedestals)
