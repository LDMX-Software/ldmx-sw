
#include "DQM/HcalInefficiencyDQM.h"

namespace dqm {
void HcalInefficiencyAnalyzer::analyze(const framework::Event &event) {
  const auto hcalSimHits = event.getCollection<ldmx::SimCalorimeterHit>(
      hcalSimHitsCollection_, hcalSimHitsPassName_);
  const auto hcalRecHits = event.getCollection<ldmx::HcalHit>(
      hcalRecHitsCollection_, hcalRecHitsPassName_);

  const int failedVeto{999};
  // Check veto for each section, combined side hcal veto
  std::vector<int> firstLayersHit{failedVeto, failedVeto, failedVeto,
                                  failedVeto, failedVeto};

  const std::vector<std::string> sectionNames{"back", "top", "bottom", "right",
                                              "left"};
  for (const auto hit : hcalRecHits) {
    const ldmx::HcalID id{hit.getID()};
    const auto section{id.section()};
    const auto z{hit.getZPos()};
    const auto layer{id.layer()};
    if (hitPassesVeto(hit, section)) {
      if (layer < firstLayersHit[section]) {
        firstLayersHit[section] = layer;
      }
    }
  }

  bool vetoedByBack{firstLayersHit[ldmx::HcalID::HcalSection::BACK] !=
                    failedVeto};
  bool vetoedByTop{firstLayersHit[ldmx::HcalID::HcalSection::TOP]};
  bool vetoedByBottom{firstLayersHit[ldmx::HcalID::HcalSection::BOTTOM]};
  bool vetoedByRight{firstLayersHit[ldmx::HcalID::HcalSection::RIGHT]};
  bool vetoedByLeft{firstLayersHit[ldmx::HcalID::HcalSection::LEFT]};
  bool vetoedBySide{vetoedByTop || vetoedByBottom || vetoedByRight ||
                    vetoedByLeft};

  for (int section{0}; section < firstLayersHit.size(); ++section) {
    const auto layer{firstLayersHit[section]};
    const auto sectionName{sectionNames[section]};
    if (layer != failedVeto) {
      histograms_.fill("inefficiency_" + sectionName, layer);
      histograms_.fill("efficiency", section);
    }
  }
  if (vetoedByBack || vetoedBySide) {
    histograms_.fill("efficiency", vetoCategories::any);
    if (vetoedByBack && vetoedBySide) {
      histograms_.fill("efficiency", vetoCategories::both);
    } else if (vetoedByBack && !vetoedBySide) {
      histograms_.fill("efficiency", vetoCategories::back_only);
    } else if (vetoedBySide && !vetoedByBack) {
      histograms_.fill("efficiency", vetoCategories::side_only);
    }
  } else {
    histograms_.fill("efficiency", vetoCategories::neither);
  }
}

void HcalInefficiencyAnalyzer::configure(

    framework::config::Parameters &parameters) {

  hcalSimHitsCollection_ =
      parameters.getParameter<std::string>("sim_coll_name");
  hcalRecHitsCollection_ =
      parameters.getParameter<std::string>("rec_coll_name");
  hcalSimHitsPassName_ = parameters.getParameter<std::string>("sim_pass_name");
  hcalRecHitsPassName_ = parameters.getParameter<std::string>("rec_pass_name");
  pe_veto_threshold = parameters.getParameter<double>("pe_veto_threshold");
  max_hit_time_ = parameters.getParameter<double>("max_hit_time");
}
} // namespace dqm

DECLARE_ANALYZER_NS(dqm, HcalInefficiencyAnalyzer);
