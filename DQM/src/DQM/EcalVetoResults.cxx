
#include "DQM/EcalVetoResults.h"

#include "Ecal/Event/EcalVetoResult.h"

namespace dqm {

void EcalVetoResults::configure(framework::config::Parameters &ps) {
  ecal_veto_name_ = ps.getParameter<std::string>("ecal_veto_name");
  ecal_veto_pass_ = ps.getParameter<std::string>("ecal_veto_pass");

  return;
}

void EcalVetoResults::analyze(const framework::Event &event) {
  auto veto{
      event.getObject<ldmx::EcalVetoResult>(ecal_veto_name_, ecal_veto_pass_)};

  histograms_.fill("bdt_disc", veto.getDisc());
  histograms_.fill("bdt_disc_log",-log(1-veto.getDisc()));
  histograms_.fill("fiducial", veto.getFiducial());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, EcalVetoResults);
