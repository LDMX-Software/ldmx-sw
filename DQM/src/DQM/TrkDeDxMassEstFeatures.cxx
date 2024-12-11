
#include "DQM/TrkDeDxMassEstFeatures.h"

#include "Recon/Event/TrackDeDxMassEstimate.h"

namespace dqm {

void TrkDeDxMassEstFeatures::configure(framework::config::Parameters &ps) {
  mass_estimate_name_ = ps.getParameter<std::string>("mass_estimate_name");
  mass_estimate_pass_ = ps.getParameter<std::string>("mass_estimate_pass");

  return;
}

void TrkDeDxMassEstFeatures::analyze(const framework::Event &event) {
  auto massEstimates{
      event.getCollection<ldmx::TrackDeDxMassEstimate>(mass_estimate_name_,
                                                       mass_estimate_pass_)};

  for (const auto &massEst : massEstimates) {
    histograms_.fill("mass_estimate", massEst.getMass());
    histograms_.fill("track_type", massEst.getTrackType());
  }

  return;
}

void TrkDeDxMassEstFeatures::onProcessStart() {
  std::vector<std::string> labels = {"Other",    // 0
                                     "Tagger",   // 1
                                     "Recoil",   // 2
                                     ""};
  TH1 *hist = histograms_.get("track_type");
  for (int ilabel{1}; ilabel < labels.size(); ++ilabel) {
    hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel - 1].c_str());
  }
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, TrkDeDxMassEstFeatures);
