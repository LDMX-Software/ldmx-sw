#include "Trigger/EcalTPSelector.h"


namespace trigger {

void EcalTPSelector::configure(framework::config::Parameters& ps) {
  tpCollName_ = ps.getParameter<std::string>("tpCollName");
  passCollName_ = ps.getParameter<std::string>("passCollName");
}

void EcalTPSelector::produce(framework::Event& event) {
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
  const ldmx::EcalHexReadout& hexReadout = getCondition<ldmx::EcalHexReadout>(
      ldmx::EcalHexReadout::CONDITIONS_OBJECT_NAME);

  if (!event.exists(tpCollName_)) return;
  auto ecalTrigDigis{
      event.getObject<ldmx::HgcrocTrigDigiCollection>(tpCollName_)};

  std::map<int, ldmx::HgcrocTrigDigiCollection > lDigis; // left
  std::map<int, ldmx::HgcrocTrigDigiCollection > rDigis; // right
  std::map<int, ldmx::HgcrocTrigDigiCollection > cDigis; // center
  std::map<int, int > lSums; // left
  std::map<int, int > rSums; // right
  std::map<int, int > cSums; // center
  for (const auto& trigDigi : ecalTrigDigis) {
    ldmx::EcalTriggerID tid(trigDigi.getId());
    int module = tid.module();
    int layer = tid.layer();
    if (module>3) {
      auto ptr = lDigis.find(layer);
      if (ptr == lDigis.end()){
        lDigis[layer] = {trigDigi};
        lSums[layer] = trigDigi.linearPrimitive();
      } else {
        lDigis[layer].push_back(trigDigi);
        lSums[layer] += trigDigi.linearPrimitive();
      }
    } else if (module>0) {
      auto ptr = rDigis.find(layer);
      if (ptr == rDigis.end()){
        rDigis[layer] = {trigDigi};
        rSums[layer] = trigDigi.linearPrimitive();
      } else {
        rDigis[layer].push_back(trigDigi);
        rSums[layer] += trigDigi.linearPrimitive();
      }
    } else {
      auto ptr = cDigis.find(layer);
      if (ptr == cDigis.end()){
        cDigis[layer] = {trigDigi};
        cSums[layer] = trigDigi.linearPrimitive();
      } else {
        cDigis[layer].push_back(trigDigi);
        cSums[layer] += trigDigi.linearPrimitive();
      }
    }
  }

  // Enforce truncation.
  // For outer modules, the energy sort is not possible
  // Instead, sort by ID to be deterministic.
  ldmx::HgcrocTrigDigiCollection passTPs;
  passTPs.reserve(ecalTrigDigis.size());
  for (auto& pair : lDigis){
    auto &digis = pair.second;
    if(digis.size()>maxOuterTPs_){
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getId() > b.getId();
                });
      digis.resize(maxCentralTPs_);
    }
    passTPs.insert( passTPs.end(), digis.begin(), digis.end() );
  }
  for (auto& pair : rDigis){
    auto &digis = pair.second;
    if(digis.size()>maxOuterTPs_){
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getId() > b.getId();
                });
      digis.resize(maxCentralTPs_);
    }
    passTPs.insert( passTPs.end(), digis.begin(), digis.end() );
  }
  // center digis, can sort by energy
  for (auto& pair : cDigis){
    auto &digis = pair.second;
    if(digis.size()>maxCentralTPs_){
      std::sort(digis.begin(), digis.end(),
                [](ldmx::HgcrocTrigDigi a, ldmx::HgcrocTrigDigi b) {
                  return a.getPrimitive() > b.getPrimitive();
                });
      digis.resize(maxCentralTPs_);
    }
    passTPs.insert( passTPs.end(), digis.begin(), digis.end() );
  }

  // collections to record (corrected to MeV)
  TrigCaloHitCollection passTrigHits;  
  for (const auto& tp : passTPs) {
    double x, y, z, e;
    decodeTP(tp, x, y, z, e);
    passTrigHits.emplace_back(x, y, z, e);
  }

  TrigEnergySumCollection passTrigSums;
  for (auto& pair : lSums){
    double e = primitiveToEnergy(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 4, e);
    passTrigSums.emplace_back(pair.first, 4, e);
  }
  for (auto& pair : rSums){
    double e = primitiveToEnergy(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 1, e);
    passTrigSums.emplace_back(pair.first, 1, e);
  }
  for (auto& pair : cSums){
    double e = primitiveToEnergy(pair.second, pair.first);
    // TrigEnergySum s(pair.first, 0, e);
    passTrigSums.emplace_back(pair.first, 0, e);
  }

  event.add(passCollName_+"Hits", passTrigHits);
  event.add(passCollName_+"Sums", passTrigSums);
  
}

double EcalTPSelector::primitiveToEnergy(int tp, int layer){
  float sie = hgc_compression_factor_ * tp *
    gain_ * mVtoMeV_;  // in MeV, before layer corrections
  return (sie / mipSiEnergy_ * layerWeights.at(layer) + sie) *
    secondOrderEnergyCorrection_;
}

void EcalTPSelector::decodeTP(ldmx::HgcrocTrigDigi tp, double &x, double &y, double &z, double &e){
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
  const ldmx::EcalHexReadout& hexReadout = getCondition<ldmx::EcalHexReadout>(
      ldmx::EcalHexReadout::CONDITIONS_OBJECT_NAME);

  ldmx::EcalTriggerID tid(tp.getId());
  const auto center_ecalID = geom.centerInTriggerCell(tid);
  hexReadout.getCellAbsolutePosition(center_ecalID,x,y,z);
  std::tie(x,y) = geom.globalPosition( tid );
  e = primitiveToEnergy(tp.linearPrimitive(), tid.layer());
}

void EcalTPSelector::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void EcalTPSelector::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void EcalTPSelector::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void EcalTPSelector::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, EcalTPSelector);
