#include "DQM/HcalVetoResults.h"
// #include "Framework/EventProcessor.h"
#include "Hcal/Event/HcalVetoResult.h"

namespace dqm {

void HcalVetoResults::configure(framework::config::Parameters &ps) {
  hcal_veto_name_ = ps.get<std::string>("hcal_veto_name");
  hcal_veto_pass_ = ps.get<std::string>("hcal_veto_pass");
  hcal_veto_passname_ = ps.get<std::string>("hcal_veto_passname");
}

void HcalVetoResults::onProcessStart() {
  std::vector<TH1 *> hists_hca_lsector = {histograms_.get("max_section")};

  // enum HcalSection { BACK = 0, TOP = 1, BOTTOM = 2, RIGHT = 3, LEFT = 4 };
  std::vector<std::string> labels_hca_lsector = {"HCAL BACK",    // 1
                                                 "HCAL TOP",     // 2
                                                 "HCAL BOTTOM",  // 3
                                                 "HCAL RIGHT",   // 4
                                                 "HCAL LEFT",    // 5
                                                 ""};

  for (int ilabel{1}; ilabel < labels_hca_lsector.size(); ++ilabel) {
    for (auto &hist : hists_hca_lsector) {
      hist->GetXaxis()->SetBinLabel(ilabel,
                                    labels_hca_lsector[ilabel - 1].c_str());
    }
  }
}

void HcalVetoResults::analyze(const framework::Event &event) {
  // Get the veto object
  auto hcal_veto{
      event.getObject<ldmx::HcalVetoResult>("HcalVeto", hcal_veto_passname_)};

  // Get variables to be plotted
  auto veto_passed = hcal_veto.passesVeto();
  auto total_pe = hcal_veto.getTotalPE();
  auto num_valid_hits = hcal_veto.getNumValidHits();
  auto max_pe_hit = hcal_veto.getMaxPEHit();
  auto max_pe = max_pe_hit.getPE();
  auto max_section = max_pe_hit.getSection();
  auto max_pos_z = max_pe_hit.getZPos();

  // ldmx_log(info) << "max_section" << max_section;

  // Fill the histograms
  histograms_.fill("max_pe", max_pe);
  histograms_.fill("total_pe", total_pe);
  histograms_.fill("num_valid_hits", num_valid_hits);
  histograms_.fill("max_section", max_section);
  histograms_.fill("max_pos_z", max_pos_z);
  histograms_.fill("veto_pass", veto_passed);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::HcalVetoResults)
