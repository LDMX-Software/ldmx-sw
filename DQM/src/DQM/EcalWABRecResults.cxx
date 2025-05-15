
#include "DQM/EcalWABRecResults.h"

#include "Ecal/Event/EcalWABResult.h"

namespace dqm {

void EcalWABRecResults::configure(framework::config::Parameters &ps) {
  ecal_WAB_rec_name_ = ps.getParameter<std::string>("ecal_WAB_rec_name");
  ecal_WAB_rec_pass_ = ps.getParameter<std::string>("ecal_WAB_rec_pass");

  return;
}

void EcalWABRecResults::analyze(const framework::Event &event) {
  auto WABRec{event.getObject<ldmx::EcalWABResult>(ecal_WAB_rec_name_,
                                                   ecal_WAB_rec_pass_)};

  histograms_.fill("ThetaDiffElectronPhoton",
                   WABRec.getRecThetaDiffElectronPhoton(),
                   WABRec.getTrueThetaDiffElectronPhoton());
  histograms_.fill("ThetaElectron", WABRec.getRecThetaElectron(),
                   WABRec.getTrueThetaElectron());
  histograms_.fill("ThetaPhoton", WABRec.getRecThetaPhoton(),
                   WABRec.getTrueThetaPhoton());
  histograms_.fill("PhiDiffElectronPhoton",
                   WABRec.getRecPhiDiffElectronPhoton(),
                   WABRec.getTruePhiDiffElectronPhoton());
  histograms_.fill("PhiElectron", WABRec.getRecPhiElectron(),
                   WABRec.getTruePhiElectron());
  histograms_.fill("PhiPhoton", WABRec.getRecPhiPhoton(),
                   WABRec.getTruePhiPhoton());
  histograms_.fill("ElectronEnergy", WABRec.getRecElectronShowerEnergy(),
                   WABRec.getTrueElectronShowerEnergy());
  histograms_.fill("PhotonEnergy", WABRec.getRecPhotonShowerEnergy(),
                   WABRec.getTruePhotonShowerEnergy());
  histograms_.fill("ElectronThetaDiff", WABRec.getTrueRecThetaDiffElectron());
  histograms_.fill("PhotonThetaDiff", WABRec.getTrueRecThetaDiffPhoton());
  histograms_.fill("ElectronPhiDiff", WABRec.getTrueRecPhiDiffElectron());
  histograms_.fill("PhotonPhiDiff", WABRec.getTrueRecPhiDiffPhoton());
  histograms_.fill("ProgressNum", WABRec.getProgressNum());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalWABRecResults);
