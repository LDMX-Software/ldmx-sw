#include "Trigger/EcalTPSelector.h"

namespace trigger {

void EcalTPSelector::configure(framework::config::Parameters& ps) {
  tpCollName_ = ps.getParameter<std::string>("tpCollName");
  passCollName_ = ps.getParameter<std::string>("passCollName");
  tp_coll_passname_ = ps.getParameter<std::string>("tp_coll_pass_name");
  tp_coll_event_passname_ =
      ps.getParameter<std::string>("tp_coll_event_passname");
}

void EcalTPSelector::produce(framework::Event& event) {
  if (!event.exists(tpCollName_, tp_coll_event_passname_)) return;
  auto ecalTrigDigis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
      tpCollName_, tp_coll_passname_)};

  std::map<int, ldmx::HgcrocTrigDigiCollection> lDigis;  // left
  std::map<int, ldmx::HgcrocTrigDigiCollection> rDigis;  // right
  std::map<int, ldmx::HgcrocTrigDigiCollection> cDigis;  // center
  std::map<int, int> lSums;                              // left
  std::map<int, int> rSums;                              // right
  std::map<int, int> cSums;                              // center
  for (const auto& trigDigi : ecalTrigDigis) {
    ldmx::EcalTriggerID tid(trigDigi.getId());
    int module_ = tid.module();
    int layer_ = tid.layer();
    if (module_ > 3) {
      auto ptr = lDigis.find(layer_);
      if (ptr == lDigis.end()) {
        lDigis[layer_] = {trigDigi};
        lSums[layer_] = trigDigi.linearPrimitive();
      } else {
        lDigis[layer_].push_back(trigDigi);
        lSums[layer_] += trigDigi.linearPrimitive();
      }
    } else if (module_ > 0) {
      auto ptr = rDigis.find(layer_);
      if (ptr == rDigis.end()) {
        rDigis[layer_] = {trigDigi};
        rSums[layer_] = trigDigi.linearPrimitive();
      } else {
        rDigis[layer_].push_back(trigDigi);
        rSums[layer_] += trigDigi.linearPrimitive();
      }
    } else {
      auto ptr = cDigis.find(layer_);
      if (ptr == cDigis.end()) {
        cDigis[layer_] = {trigDigi};
        cSums[layer_] = trigDigi.linearPrimitive();
      } else {
        cDigis[layer_].push_back(trigDigi);
        cSums[layer_] += trigDigi.linearPrimitive();
      }
    }
  }

  // Enforce truncation.
  // For outer modules, the energy sort is not possible
  // Instead, sort by ID to be deterministic.
  ldmx::HgcrocTrigDigiCollection passTPs;
  passTPs.reserve(ecalTrigDigis.size());
  for (auto& pair : lDigis) {
    auto& digis = pair.second;
    if (digis.size() > maxOuterTPs_) {
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getId() > b.getId();
                });
      digis.resize(maxCentralTPs_);
    }
    passTPs.insert(passTPs.end(), digis.begin(), digis.end());
  }
  for (auto& pair : rDigis) {
    auto& digis = pair.second;
    if (digis.size() > maxOuterTPs_) {
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getId() > b.getId();
                });
      digis.resize(maxCentralTPs_);
    }
    passTPs.insert(passTPs.end(), digis.begin(), digis.end());
  }
  // center digis, can sort by energy
  for (auto& pair : cDigis) {
    auto& digis = pair.second;
    if (digis.size() > maxCentralTPs_) {
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getPrimitive() > b.getPrimitive();
                });
      digis.resize(maxCentralTPs_);
    }
    passTPs.insert(passTPs.end(), digis.begin(), digis.end());
  }

  // collections to record (corrected to MeV)
  std::vector<TrigCaloHit> passTrigHits;
  for (const auto& tp : passTPs) {
    double x_, y_, z_, e;
    decodeTP(tp, x_, y_, z_, e);
    passTrigHits.emplace_back(x_, y_, z_, e);

    ldmx::EcalTriggerID tid(tp.getId());
    passTrigHits.back().setLayer(tid.layer());
    passTrigHits.back().setModule(tid.module());
  }

  TrigEnergySumCollection passTrigSums;
  ecalTpToE cvt;
  for (auto& pair : lSums) {
    double e = cvt.calc(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 4, e);
    passTrigSums.emplace_back(pair.first, 4, e);
  }
  for (auto& pair : rSums) {
    double e = cvt.calc(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 1, e);
    passTrigSums.emplace_back(pair.first, 1, e);
  }
  for (auto& pair : cSums) {
    double e = cvt.calc(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 0, e);
    passTrigSums.emplace_back(pair.first, 0, e);
  }

  event.add(passCollName_ + "Hits", passTrigHits);
  event.add(passCollName_ + "Sums", passTrigSums);
}

// double EcalTPSelector::primitiveToEnergy(int tp, int layer_){
//   float sie = hgc_compression_factor_ * tp *
//     gain_ * mVtoMeV_;  // in MeV, before layer_ corrections
//   return (sie / mipSiEnergy_ * layerWeights.at(layer_) + sie) *
//     second_order_energy_correction_ * adHoc_;
// }

void EcalTPSelector::decodeTP(ldmx::HgcrocTrigDigi tp, double& x_, double& y_,
                              double& z_, double& e) {
  ldmx::EcalTriggerID tid(tp.getId());
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
  // const auto center_ecalID = geom.centerInTriggerCell(tid);
  //  const ldmx::EcalGeometry& hexReadout = getCondition<ldmx::EcalGeometry>(
  //  ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);
  // hexReadout.getCellAbsolutePosition(center_ecalID,x_,y_,z_);
  std::tie(x_, y_, z_) = geom.globalPosition(tid);
  // e = primitiveToEnergy(tp.linearPrimitive(), tid.layer());
  ecalTpToE cvt;
  e = cvt.calc(tp.linearPrimitive(), tid.layer());
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::EcalTPSelector);
