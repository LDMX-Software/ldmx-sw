
#include "DQM/HCalDQM.h"

#include "DetDescr/HcalID.h"
#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalVetoResult.h"

namespace dqm {

HCalDQM::HCalDQM(const std::string &name, framework::Process &process)
    : framework::Analyzer(name, process) {}

void HCalDQM::configure(framework::config::Parameters &ps) {
  rec_coll_name_ = ps.getParameter<std::string>("rec_coll_name");
  rec_pass_name_ = ps.getParameter<std::string>("rec_pass_name");
  pe_veto_threshold = ps.getParameter<double>("pe_threshold");
  section_ = ps.getParameter<int>("section");
}

void HCalDQM::analyze(const framework::Event &event) {
  // Get the collection of HCalDQM digitized hits if the exists
  const auto &hcalHits{
      event.getCollection<ldmx::HcalHit>(rec_coll_name_, rec_pass_name_)};

  const auto &geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  float totalPE{0};
  float maxPE{-1};
  float maxPETime{-1};
  float E{0};
  float totalE{0};
  int vetoableHitMultiplicity{0};
  int hitMultiplicity{0};

  auto passesVeto = [&](const ldmx::HcalHit &hit, int section) {
    constexpr double max_time{50};
    if (hit.getPE() < pe_veto_threshold) {
      return true;
    }
    if (section == ldmx::HcalID::HcalSection::BACK && hit.getMinPE() < 1) {
      return true;
    }
    return false;
  };
  for (const ldmx::HcalHit &hit : hcalHits) {
    ldmx::HcalID id(hit.getID());
    const auto section{id.section()};
    const auto layer{id.layer()};
    const auto strip{id.strip()};
    if (section != section_ && section_ != -1) {
      continue;
    }

    if (hit.isNoise()) {
      std::cout << "Found a noise hit!" << std::endl;
      char c;
      std::cin >> c;
    }
    if (passesVeto(hit, section)) {
      hitMultiplicity++;
    } else {
      hitMultiplicity++;
      vetoableHitMultiplicity++;
    }
    const auto pe{hit.getPE()};
    const auto t{hit.getTime()};
    const auto e{hit.getEnergy()};
    const auto x{hit.getZPos()};
    const auto z{hit.getZPos()};
    switch (section) {
    case ldmx::HcalID::HcalSection::BACK:
    case ldmx::HcalID::HcalSection::TOP:
    case ldmx::HcalID::HcalSection::BOTTOM:
    case ldmx::HcalID::HcalSection::LEFT:
    case ldmx::HcalID::HcalSection::RIGHT:
    }

    totalE += e;
    totalPE += pe;

    if (pe > maxPE) {
      maxPE = pe;
      maxPETime = t;
    }
    histograms_.fill("layer:strip", layer, strip);
    histograms_.fill("pe", pe);
    histograms_.fill("hit_time", t);
    histograms_.fill("layer", layer);
    histograms_.fill("noise", hit.isNoise());
    histograms_.fill("energy", e);
    histograms_.fill("hit_z", z);
  }
  histograms_.fill("total_energy", totalE);
  histograms_.fill("total_pe", totalPE);
  histograms_.fill("max_pe", maxPE);
  histograms_.fill("max_pe_time", maxPETime);
  histograms_.fill("hit_multiplicity", hitMultiplicity);
  histograms_.fill("vetoable_hit_multiplicity", vetoableHitMultiplicity);

  // float totalPE{0}, total_back_pe{0}, minTime{999}, minTimePE{-1};
  // float maxPE{-1}, maxPETime{-1};
  // std::vector<float> total_section_pe(5);
  // // Loop through all HCal hits in the event
  // // Get non-noise generated hits into new vector for sorting
  // for (const ldmx::HcalHit &hit : hcalHits) {
  //   ldmx::HcalID id(hit.getID());

  //   std::string section{"side"};
  //   switch (id.section()) {
  //   case ldmx::HcalID::HcalSection::BACK:
  //     section = "back";
  //     break;
  //   case ldmx::HcalID::HcalSection::TOP:
  //     section = "top";
  //     break;
  //   case ldmx::HcalID::HcalSection::BOTTOM:
  //     section = "bottom";
  //     break;
  //   case ldmx::HcalID::HcalSection::LEFT:
  //     section = "left";
  //     break;
  //   case ldmx::HcalID::HcalSection::RIGHT:
  //     section = "right";
  //     break;
  //   }
  //   histograms_.fill("pe", hit.getPE());
  //   histograms_.fill("hit_time", hit.getTime());
  //   histograms_.fill("layer", id.layer());
  //   histograms_.fill(section + "_pe", hit.getPE());
  //   histograms_.fill(section + "_layer", id.layer());

  //   totalPE += hit.getPE();
  //   total_section_pe[id.section()] += hit.getPE();
  //   // histograms_.fill(section + "_pe:layer", hit.getPE(), id.layer());
  //   // histograms_.fill(section + "_layer:strip", id.layer(), id.strip());

  //   /**
  //    * Get earliest (min time) non-noise (time > -999.ns) hit
  //    * above the PE veto threshold.
  //    */
  //   if (hit.getTime() > -999. and hit.getPE() > pe_veto_threshold and
  //       hit.getTime() < minTime) {
  //     minTime = hit.getTime();
  //     minTimePE = hit.getPE();
  //   }

  //   /**
  //    * Get PE value and time of hit
  //    * with maximum PE deposited in event.
  //    */
  //   if (hit.getPE() > maxPE) {
  //     maxPE = hit.getPE();
  //     maxPETime = hit.getTime();
  //   }
  // }

  // // let's fill our once-per-event histograms
  // histograms_.fill("n_hits", hcalHits.size());
  // histograms_.fill("total_pe", totalPE);
  // histograms_.fill("back_total_pe", total_section_pe[0]);
  // histograms_.fill("top_total_pe", total_section_pe[1]);
  // histograms_.fill("bottom_total_pe", total_section_pe[2]);
  // histograms_.fill("left_total_pe", total_section_pe[3]);
  // histograms_.fill("right_total_pe", total_section_pe[4]);
  // histograms_.fill("max_pe", maxPE);
  // histograms_.fill("hit_time_max_pe", maxPETime);
  // histograms_.fill("max_pe:time", maxPE, maxPETime);
  // histograms_.fill("min_time_hit_above_thresh", minTime);
  // histograms_.fill("min_time_hit_above_thresh:pe", minTimePE, minTime);
}

} // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalDQM)
