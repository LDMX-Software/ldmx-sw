
#include "DQM/HcalInefficiencyDQM.h"

namespace dqm {
void HcalInefficiencyAnalyzer::analyze(const framework::Event &event) {
  const auto hcal_sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
      hcalSimHitsCollection_, hcalSimHitsPassName_);
  const auto hcal_rec_hits = event.getCollection<ldmx::HcalHit>(
      hcalRecHitsCollection_, hcalRecHitsPassName_);

  const int failed_veto{999};
  // Check veto for each section, combined side hcal veto
  std::vector<int> first_layers_hit{failed_veto, failed_veto, failed_veto,
                                  failed_veto, failed_veto};

  const std::vector<std::string> section_names{"back", "top", "bottom", "right",
                                              "left"};
  for (const auto &hit : hcal_rec_hits) {
    const ldmx::HcalID id{static_cast<ldmx::DetectorID::RawValue>(hit.getID())};
    const auto section{id.section()};
    const auto layer{id.layer()};
    if (hitPassesVeto(hit, section)) {
      if (layer < first_layers_hit[section]) {
        first_layers_hit[section] = layer;
      }
    }
  }

  bool vetoed_by_back{first_layers_hit[ldmx::HcalID::HcalSection::BACK] !=
                    failed_veto};
  bool vetoed_by_top{first_layers_hit[ldmx::HcalID::HcalSection::TOP] !=
                   failed_veto};
  bool vetoed_by_bottom{first_layers_hit[ldmx::HcalID::HcalSection::BOTTOM] !=
                      failed_veto};
  bool vetoed_by_right{first_layers_hit[ldmx::HcalID::HcalSection::RIGHT] !=
                     failed_veto};
  bool vetoed_by_left{first_layers_hit[ldmx::HcalID::HcalSection::LEFT] !=
                    failed_veto};
  bool vetoed_by_side{vetoed_by_top || vetoed_by_bottom || vetoed_by_right ||
                    vetoed_by_left};

  for (int section{0}; section < first_layers_hit.size(); ++section) {
    const auto layer{first_layers_hit[section]};
    const auto section_name{section_names[section]};
    if (layer != failed_veto) {
      histograms_.fill("inefficiency_" + section_name, layer);
      histograms_.fill("efficiency", section);
    }
  }
  if (vetoed_by_back || vetoed_by_side) {
    histograms_.fill("efficiency", vetoCategories::any);
    if (vetoed_by_back && vetoed_by_side) {
      histograms_.fill("efficiency", vetoCategories::both);
    } else if (vetoed_by_back && !vetoed_by_side) {
      histograms_.fill("efficiency", vetoCategories::back_only);
    } else if (vetoed_by_side && !vetoed_by_back) {
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
}  // namespace dqm

DECLARE_ANALYZER(dqm::HcalInefficiencyAnalyzer);
