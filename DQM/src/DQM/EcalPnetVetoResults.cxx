
#include "DQM/EcalPnetVetoResults.h"

#include "Ecal/Event/EcalVetoResult.h"

namespace dqm {

void EcalPnetVetoResults::configure(framework::config::Parameters &ps) {
  ecal_pnet_veto_name_ = ps.getParameter<std::string>("ecal_pnet_veto_name");
  ecal_pnet_veto_pass_ = ps.getParameter<std::string>("ecal_pnet_veto_pass");

  return;
}

void EcalPnetVetoResults::analyze(const framework::Event &event) {
  auto veto{event.getObject<ldmx::EcalVetoResult>(ecal_pnet_veto_name_,
                                                  ecal_pnet_veto_pass_)};

  histograms_.fill("pnet_disc", veto.getDisc());
  histograms_.fill("pnet_disc_log", -std::log10(1 - veto.getDisc()));
  histograms_.fill("pnet_pass", veto.passesVeto());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalPnetVetoResults);
