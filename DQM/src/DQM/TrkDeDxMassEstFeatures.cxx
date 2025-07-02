
#include "DQM/TrkDeDxMassEstFeatures.h"

#include "Recon/Event/TrackDeDxMassEstimate.h"

namespace dqm {

void TrkDeDxMassEstFeatures::configure(framework::config::Parameters &ps) {
  mass_estimate_name_ = ps.getParameter<std::string>("mass_estimate_name");
  mass_estimate_pass_ = ps.getParameter<std::string>("mass_estimate_pass");

  return;
}

void TrkDeDxMassEstFeatures::analyze(const framework::Event &event) {
  auto mass_estimates_{event.getCollection<ldmx::TrackDeDxMassEstimate>(
      mass_estimate_name_, mass_estimate_pass_)};

  for (const auto &mass_est : mass_estimates_) {
    auto momentum = mass_est.getMomentum();
    histograms_.fill("momentum:harmonic_mean_dedx", momentum, mass_est.getIh());
    if (momentum < 2000.)
      histograms_.fill("momentum_low:harmonic_mean_dedx", momentum,
                       mass_est.getIh());
    histograms_.fill("harmonic_mean_dedx", mass_est.getIh());
    histograms_.fill("mass_estimate", mass_est.getMass());
    if (momentum < 1000.) {
      histograms_.fill("mass_estimate_low_p", mass_est.getMass());
    }
    if (momentum < 500.) {
      histograms_.fill("mass_estimate_very_low_p", mass_est.getMass());
      if ((mass_est.getPdgId() == 11) || (mass_est.getPdgId() == -11)) {
        histograms_.fill("mass_estimate_very_low_p_electron",
                         mass_est.getMass());
      }
      if ((mass_est.getPdgId() == 211) || (mass_est.getPdgId() == -211)) {
        histograms_.fill("mass_estimate_very_low_p_pion", mass_est.getMass());
      }
      if ((mass_est.getPdgId() == 321) || (mass_est.getPdgId() == -321)) {
        histograms_.fill("mass_estimate_very_low_p_kaon", mass_est.getMass());
      }
      if ((mass_est.getPdgId() == 2212) || (mass_est.getPdgId() == -2212)) {
        histograms_.fill("mass_estimate_very_low_p_proton", mass_est.getMass());
      }
    }
    histograms_.fill("track_type", mass_est.getTrackType());
  }

  return;
}

void TrkDeDxMassEstFeatures::onProcessStart() {
  std::vector<std::string> labels = {"Other",   // 0
                                     "Tagger",  // 1
                                     "Recoil",  // 2
                                     ""};
  TH1 *hist = histograms_.get("track_type");
  for (int ilabel{1}; ilabel < labels.size(); ++ilabel) {
    hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel - 1].c_str());
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::TrkDeDxMassEstFeatures);
