
#include "DQM/EcalWABRecResults.h"

#include "Ecal/Event/EcalWABResult.h"

namespace dqm {

void EcalWABRecResults::configure(framework::config::Parameters &ps) {
  ecal_WAB_rec_name_ = ps.getParameter<std::string>("ecal_WAB_rec_name");
  ecal_WAB_rec_pass_ = ps.getParameter<std::string>("ecal_WAB_rec_pass");

  return;
}

void EcalWABRecResults::analyze(const framework::Event &event) {
  auto WABRec{
      event.getObject<ldmx::EcalWABResult>(ecal_WAB_rec_name_, ecal_WAB_rec_pass_)};

  histograms_.fill("ThetaElectron", WABRec.getRecThetaElectron(), WABRec.getTrueThetaElectron());
  histograms_.fill("ThetaPhoton", WABRec.getRecThetaPhoton(), WABRec.getTrueThetaPhoton());
  histograms_.fill("PhiDiffElectronPhoton", WABRec.getRecPhiDiffElectronPhoton(), WABRec.getTruePhiDiffElectronPhoton());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, EcalWABRecResults);
