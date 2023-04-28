
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
}

} // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalDQM)
