
#include "DQM/EcalWABRecResults.h"

#include "Ecal/Event/EcalWABResult.h"

namespace dqm {

void EcalWABRecResults::configure(framework::config::Parameters &ps) {
  ecal_WAB_rec_name_ = ps.get<std::string>("ecal_WAB_rec_name");
  ecal_WAB_rec_pass_ = ps.get<std::string>("ecal_WAB_rec_pass");

  return;
}

void EcalWABRecResults::analyze(const framework::Event &event) {
  auto wab_rec{event.getObject<ldmx::EcalWABResult>(ecal_WAB_rec_name_,
                                                    ecal_WAB_rec_pass_)};

  histograms_.fill("ThetaDiffElectronPhoton",
                   wab_rec.getRecThetaDiffElectronPhoton(),
                   wab_rec.getTrueThetaDiffElectronPhoton());
  histograms_.fill("ThetaElectron", wab_rec.getRecThetaElectron(),
                   wab_rec.getTrueThetaElectron());
  histograms_.fill("ThetaPhoton", wab_rec.getRecThetaPhoton(),
                   wab_rec.getTrueThetaPhoton());
  histograms_.fill("PhiDiffElectronPhoton",
                   wab_rec.getRecPhiDiffElectronPhoton(),
                   wab_rec.getTruePhiDiffElectronPhoton());
  histograms_.fill("PhiElectron", wab_rec.getRecPhiElectron(),
                   wab_rec.getTruePhiElectron());
  histograms_.fill("PhiPhoton", wab_rec.getRecPhiPhoton(),
                   wab_rec.getTruePhiPhoton());
  histograms_.fill("ElectronEnergy", wab_rec.getRecElectronShowerEnergy(),
                   wab_rec.getTrueElectronShowerEnergy());
  histograms_.fill("PhotonEnergy", wab_rec.getRecPhotonShowerEnergy(),
                   wab_rec.getTruePhotonShowerEnergy());
  histograms_.fill("ElectronThetaDiff", wab_rec.getTrueRecThetaDiffElectron());
  histograms_.fill("PhotonThetaDiff", wab_rec.getTrueRecThetaDiffPhoton());
  histograms_.fill("ElectronPhiDiff", wab_rec.getTrueRecPhiDiffElectron());
  histograms_.fill("PhotonPhiDiff", wab_rec.getTrueRecPhiDiffPhoton());
  histograms_.fill("ProgressNum", wab_rec.getProgressNum());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalWABRecResults);
