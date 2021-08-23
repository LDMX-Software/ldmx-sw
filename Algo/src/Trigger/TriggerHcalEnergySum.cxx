#include "Trigger/TriggerHcalEnergySum.h"

#include "DetDescr/HcalGeometry.h"
#include "Hcal/HcalTriggerGeometry.h"
#include "Recon/Event/CalorimeterHit.h"
#include "Recon/Event/CaloTrigPrim.h"
#include "Trigger/Event/TrigEnergySum.h"

namespace trigger {

void TriggerHcalEnergySum::configure(framework::config::Parameters& ps) {}

void TriggerHcalEnergySum::produce(framework::Event& event) {
  // mV/ADC: 1.2
  // MeV/MIP: 4.66
  // PE/MIP: 68 (summed over BOTH ends, based on 1808.05219, p38)
  // mV/PE: 5
  // mV/MeV: 72.961 (= 5*68/4.66)
  const float mV_per_adc = 1.2;
  // adc gain
  const float pe_per_adc = mV_per_adc / 5;
  const float MeV_per_adc = mV_per_adc / 72.961;
  // const float samp_frac = 371/4e3; // ad-hoc, from a 4 GeV neutron sample

  // interaction length in Fe ('steel') = 16.77 cm (132.1 g/cm2)
  // polystyrene = 77.07 cm (81.7 g/cm2)
  // back hcal is 20mm bar, 25mm absorber
  const float had_samp_frac = (20/77.07)/(20/77.07 + 25/16.77); // 0.148266
  const float em_samp_frac = (20/41.31)/(20/41.31 + 25/1.757); // 0.032906
  const float samp_frac = (em_samp_frac + 2*had_samp_frac)/3; // 0.109813
  const float attenuation = exp(-1/5.); // 5m attenuation length, 1m half-bar
  
  const ldmx::HcalGeometry& hcalGeom =
    getCondition<ldmx::HcalGeometry>(ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  const hcal::HcalTriggerGeometry& trigGeom =
    getCondition<hcal::HcalTriggerGeometry>(hcal::HcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

  if (!event.exists("hcalOneEndedTrigQuads")) return;
  auto oneEndedQuads{event.getObject<ldmx::CaloTrigPrimCollection>("hcalOneEndedTrigQuads")};

  //
  // sum bar ends to produce the combined quads
  std::map<int, ldmx::CaloTrigPrim> twoEndedQuadMap;
  for (auto oneEndedQuad : oneEndedQuads) {
    const ldmx::HcalTriggerID end_id(oneEndedQuad.getId());
    ldmx::HcalTriggerID combo_id(end_id.section(), end_id.layer(), end_id.superstrip(), 2);
    auto ptr = twoEndedQuadMap.find(combo_id.raw());
    if (ptr == twoEndedQuadMap.end()){
      twoEndedQuadMap[combo_id.raw()] = oneEndedQuad;
    } else {
      ptr->second.setPrimitive( ptr->second.getPrimitive() + oneEndedQuad.getPrimitive() );
    }
  }
  ldmx::CaloTrigPrimCollection twoEndedQuads;
  for (auto p : twoEndedQuadMap) twoEndedQuads.push_back( p.second );
  event.add("hcalTrigQuads", twoEndedQuads);

  //
  // Produce the layer-by-layer energy sums  
  const unsigned int LayerMax = 50;
  ldmx::TrigEnergySumCollection layerSums;
  layerSums.resize(LayerMax);
  for(int i=0; i<LayerMax;i++){
    layerSums[i].setLayer(i);
  }

  int total_adc = 0;
  for (auto p : twoEndedQuadMap) {
    auto tp = p.second;
    int adc = tp.getPrimitive();
    total_adc += adc;
    ldmx::HcalTriggerID combo_id(tp.getId());
    int ilayer= combo_id.layer();
    if(ilayer >= layerSums.size()){
      std::cout << "[TriggerHcalEnergySum.cxx] Warning(!), layer "
                <<ilayer<<" is out-of-bounds.\n";
      continue;
    }
    layerSums[ilayer].setHwEnergy(adc + layerSums[ilayer].hwEnergy());
  }
  event.add("hcalTrigQuadLayerSums", layerSums);

  // Also store total energy for now
  ldmx::TrigEnergySum totalSum;
  totalSum.setLayer(-1);
  totalSum.setHwEnergy(total_adc);
  event.add("hcalTrigQuadSum", totalSum);

}

void TriggerHcalEnergySum::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void TriggerHcalEnergySum::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void TriggerHcalEnergySum::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void TriggerHcalEnergySum::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TriggerHcalEnergySum);
