
#include "DQM/HCalDQM.h"
namespace dqm {

HCalDQM::HCalDQM(const std::string &name, framework::Process &process)
    : framework::Analyzer(name, process) {}

void HCalDQM::configure(framework::config::Parameters &ps) {
  rec_coll_name_ = ps.get<std::string>("rec_coll_name");
  rec_pass_name_ = ps.get<std::string>("rec_pass_name");
  sim_coll_name_ = ps.get<std::string>("sim_coll_name");
  sim_pass_name_ = ps.get<std::string>("sim_pass_name");
  pe_veto_threshold_ = ps.get<double>("pe_veto_threshold");
  section_ = ps.get<int>("section");
  max_hit_time_ = ps.get<double>("max_hit_time");
}

void HCalDQM::analyze(const framework::Event &event) {
  // Get the collection of HCalDQM digitized hits if the exists
  const auto &hcal_hits{
      event.getCollection<ldmx::HcalHit>(rec_coll_name_, rec_pass_name_)};

  const auto &hcal_sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      sim_coll_name_, sim_pass_name_)};
  analyzeSimHits(hcal_sim_hits);
  analyzeRecHits(hcal_hits);
}
void HCalDQM::analyzeSimHits(const std::vector<ldmx::SimCalorimeterHit> &hits) {
  const auto &geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  std::map<ldmx::HcalID, double> sim_energy_per_bar;
  int hit_multiplicity{0};

  for (const auto &hit : hits) {
    ldmx::HcalID id(hit.getID());
    if (skipHit(id)) {
      continue;
    }
    const auto energy{hit.getEdep()};
    if (sim_energy_per_bar.count(id) == 0) {
      sim_energy_per_bar[id] = energy;
    } else {
      sim_energy_per_bar[id] += energy;
    }
    const auto orientation{geometry.getScintillatorOrientation(id)};
    const auto layer{id.layer()};
    const auto strip{id.strip()};
    const auto pos{hit.getPosition()};
    const auto x{pos[0]};
    const auto y{pos[1]};
    const auto z{pos[2]};
    const auto t{hit.getTime()};
    hit_multiplicity++;
    histograms_.fill("sim_hit_time", t);
    histograms_.fill("sim_layer", layer);
    histograms_.fill("sim_layer:strip", layer, strip);
    histograms_.fill("sim_energy", energy);
    switch (orientation) {
      case ldmx::HcalGeometry::ScintillatorOrientation::horizontal:
        histograms_.fill("sim_along_x", x);
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::vertical:
        histograms_.fill("sim_along_y", y);
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::depth:
        histograms_.fill("sim_along_z", z);
        break;
    }
  }

  histograms_.fill("sim_hit_multiplicity", hit_multiplicity);
  histograms_.fill("sim_num_bars_hit", sim_energy_per_bar.size());

  double total_energy{0};
  for (const auto [id, energy] : sim_energy_per_bar) {
    histograms_.fill("sim_energy_per_bar", energy);
    total_energy += energy;
  }
  histograms_.fill("sim_total_energy", total_energy);
}
void HCalDQM::analyzeRecHits(const std::vector<ldmx::HcalHit> &hits) {
  const auto &geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  float total_pe{0};
  float max_pe{-1};
  float max_pe_time{-1};
  float total_e{0};
  int vetoable_hit_multiplicity{0};
  int hit_multiplicity{0};

  for (const ldmx::HcalHit &hit : hits) {
    ldmx::HcalID id(hit.getID());
    const auto orientation{geometry.getScintillatorOrientation(id)};
    const auto section{id.section()};
    const auto layer{id.layer()};
    const auto strip{id.strip()};
    if (skipHit(id)) {
      continue;
    }

    if (hit.isNoise()) {
      histograms_.fill("noise", 1);
    } else {
      histograms_.fill("noise", 0);
    }
    if (hitPassesVeto(hit, section)) {
      hit_multiplicity++;
    } else {
      hit_multiplicity++;
      vetoable_hit_multiplicity++;
    }
    const auto pe{hit.getPE()};
    const auto t{hit.getTime()};
    const auto e{hit.getEnergy()};
    const auto x{hit.getXPos()};
    const auto y{hit.getYPos()};
    const auto z{hit.getZPos()};
    switch (orientation) {
      case ldmx::HcalGeometry::ScintillatorOrientation::horizontal:
        histograms_.fill("along_x", x);
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::vertical:
        histograms_.fill("along_y", y);
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::depth:
        histograms_.fill("along_z", z);
        break;
    }
    total_e += e;
    total_pe += pe;

    if (pe > max_pe) {
      max_pe = pe;
      max_pe_time = t;
    }
    histograms_.fill("layer:strip", layer, strip);
    histograms_.fill("pe", pe);
    histograms_.fill("hit_time", t);
    histograms_.fill("layer", layer);
    histograms_.fill("noise", hit.isNoise());
    histograms_.fill("energy", e);
    histograms_.fill("hit_z", z);
  }
  histograms_.fill("total_energy", total_e);
  histograms_.fill("total_pe", total_pe);
  histograms_.fill("max_pe", max_pe);
  histograms_.fill("max_pe_time", max_pe_time);
  histograms_.fill("hit_multiplicity", hit_multiplicity);
  histograms_.fill("vetoable_hit_multiplicity", vetoable_hit_multiplicity);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::HCalDQM)
