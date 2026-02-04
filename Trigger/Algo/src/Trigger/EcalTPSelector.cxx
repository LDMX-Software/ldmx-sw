#include "Trigger/EcalTPSelector.h"

namespace trigger {

void EcalTPSelector::configure(framework::config::Parameters& ps) {
  tp_coll_name_ = ps.get<std::string>("tpCollName");
  pass_coll_name_ = ps.get<std::string>("passCollName");
  tp_coll_passname_ = ps.get<std::string>("tp_coll_pass_name");
  tp_coll_event_passname_ = ps.get<std::string>("tp_coll_event_passname");
}

void EcalTPSelector::produce(framework::Event& event) {
  // if (!event.exists(tp_coll_name_, tp_coll_event_passname_)) return;
  auto ecal_trig_digis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
      tp_coll_name_, tp_coll_passname_)};

  // std::map<int, ldmx::HgcrocTrigDigiCollection> l_digis;  // left
  // std::map<int, ldmx::HgcrocTrigDigiCollection> r_digis;  // right
  std::map<int, ldmx::HgcrocTrigDigiCollection> c_digis;  // center
  std::map<int, ldmx::HgcrocTrigDigiCollection> o_digis;  // outer
  // std::map<int, int> l_sums;                              // left
  // std::map<int, int> r_sums;                              // right
  std::map<int, int> c_sums;                              // center
  std::map<int, int> o_sums;                              // outer
  for (const auto& trig_digi : ecal_trig_digis) {
    ldmx::EcalTriggerID tid(trig_digi.getId());
    int module = tid.module();
    int layer = tid.layer();

    if (module > 0) {
      auto ptr = o_digis.find(layer);
      if (ptr == o_digis.end()) {
        o_digis[layer] = {trig_digi};
        o_sums[layer] = trig_digi.linearPrimitive();
      } else {
        o_digis[layer].push_back(trig_digi);
        o_sums[layer] += trig_digi.linearPrimitive();
      }
    } else {
      auto ptr = c_digis.find(layer);
      if (ptr == c_digis.end()) {
        c_digis[layer] = {trig_digi};
        c_sums[layer] = trig_digi.linearPrimitive();
      } else {
        c_digis[layer].push_back(trig_digi);
        c_sums[layer] += trig_digi.linearPrimitive();
      }
    }
  }

  // Enforce truncation.
  // For outer modules, the energy sort is not possible
  // Instead, sort by ID to be deterministic.
  ldmx::HgcrocTrigDigiCollection pass_t_ps;
  pass_t_ps.reserve(ecal_trig_digis.size());
  for (auto& pair : o_digis) {
    auto& digis = pair.second;
    if (digis.size() > max_outer_t_ps_) {
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getId() > b.getId();
                });
      digis.resize(max_central_t_ps_);
    }
    pass_t_ps.insert(pass_t_ps.end(), digis.begin(), digis.end());
  }
  // for (auto& pair : r_digis) {
  //   auto& digis = pair.second;
  //   if (digis.size() > max_outer_t_ps_) {
  //     std::sort(digis.begin(), digis.end(),
  //               [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
  //                 return a.getId() > b.getId();
  //               });
  //     digis.resize(max_central_t_ps_);
  //   }
  //   pass_t_ps.insert(pass_t_ps.end(), digis.begin(), digis.end());
  // }
  // center digis, can sort by energy
  for (auto& pair : c_digis) {
    auto& digis = pair.second;
    if (digis.size() > max_central_t_ps_) {
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getPrimitive() > b.getPrimitive();
                });
      digis.resize(max_central_t_ps_);
    }
    pass_t_ps.insert(pass_t_ps.end(), digis.begin(), digis.end());
  }

  // collections to record (corrected to MeV)
  std::vector<TrigCaloHit> pass_trig_hits;
  for (const auto& tp : pass_t_ps) {
    double x, y, z, e;
    decodeTP(tp, x, y, z, e);
    pass_trig_hits.emplace_back(x, y, z, e);

    ldmx::EcalTriggerID tid(tp.getId());
    pass_trig_hits.back().setLayer(tid.layer());
    pass_trig_hits.back().setModule(tid.module());
  }

  TrigEnergySumCollection pass_trig_sums;
  EcalTpToE cvt;
  for (auto& pair : o_sums) {
    double e = cvt.calc(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 4, e);
    pass_trig_sums.emplace_back(pair.first, 1, e);
  }
  // for (auto& pair : r_sums) {
  //   double e = cvt.calc(pair.second, pair.first);
  //   // TrigEnergySum s(pair.first, 1, e);
  //   pass_trig_sums.emplace_back(pair.first, 1, e);
  // }
  for (auto& pair : c_sums) {
    double e = cvt.calc(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 0, e);
    pass_trig_sums.emplace_back(pair.first, 0, e);
  }
  event.add(pass_coll_name_ + "Hits", pass_trig_hits);
  event.add(pass_coll_name_ + "Sums", pass_trig_sums);
}

// double EcalTPSelector::primitiveToEnergy(int tp, int layer){
//   float sie = hgc_compression_factor_ * tp *
//     gain_ * m_vto_me_v_;  // in MeV, before layer corrections
//   return (sie / mip_si_energy_ * layerWeights.at(layer) + sie) *
//     second_order_energy_correction_ * ad_hoc_;
// }

void EcalTPSelector::decodeTP(ldmx::HgcrocTrigDigi tp, double& x, double& y,
                              double& z, double& e) {
  ldmx::EcalTriggerID tid(tp.getId());
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
  // const auto center_ecalID = geom.centerInTriggerCell(tid);
  //  const ldmx::EcalGeometry& hexReadout = getCondition<ldmx::EcalGeometry>(
  //  ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);
  // hexReadout.getCellAbsolutePosition(center_ecalID,x,y,z);
  std::tie(x, y, z) = geom.globalPosition(tid);
  // e = primitiveToEnergy(tp.linearPrimitive(), tid.layer());
  EcalTpToE cvt;
  e = cvt.calc(tp.linearPrimitive(), tid.layer());
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::EcalTPSelector);
