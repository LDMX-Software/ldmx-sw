#include "Trigger/TrigHcalEnergySum.h"

#include "DetDescr/HcalGeometry.h"
#include "Hcal/HcalTriggerGeometry.h"
#include "Recon/Event/CaloTrigPrim.h"
#include "Recon/Event/CalorimeterHit.h"
#include "Trigger/Event/TrigEnergySum.h"

namespace trigger {

void TrigHcalEnergySum::configure(framework::config::Parameters& ps) {
  in_proc_ = ps.get<std::string>("input_proc");
  quad_coll_name_ = ps.get<std::string>("quad_coll_name");
  combined_quad_coll_name_ = ps.get<std::string>("combined_quad_coll_name");
}
void TrigHcalEnergySum::produce(framework::Event& event) {
  // mV/ADC: 1.2
  // MeV/MIP: 4.66
  // PE/MIP: 68 (summed over BOTH ends, based on 1808.05219, p38)
  // mV/PE: 5
  // mV/MeV: 72.961 (= 5*68/4.66)
  //  const float mV_per_adc = 1.2;
  // adc gain
  // these are unused, should they be? FIXME
  // const float pe_per_adc = mV_per_adc / 5;
  // const float MeV_per_adc = mV_per_adc / 72.961;
  // const float samp_frac = 371/4e3; // ad-hoc, from a 4 GeV neutron sample

  // interaction length in Fe ('steel') = 16.77 cm (132.1 g/cm2)
  // polystyrene = 77.07 cm (81.7 g/cm2)
  // back hcal is 20mm bar, 25mm absorber
  // these are unused, should they be? FIXME
  // const float had_samp_frac =
  //    (20 / 77.07) / (20 / 77.07 + 25 / 16.77);  // 0.148266
  // const float em_samp_frac =
  //    (20 / 41.31) / (20 / 41.31 + 25 / 1.757);                    // 0.032906
  // const float samp_frac = (em_samp_frac + 2 * had_samp_frac) / 3;  //
  // 0.109813 const float attenuation = exp(-1 / 5.);  // 5m attenuation length,
  // 1m half-bar

  // for(auto t : event.searchProducts("","","")) std::cout << t.name() << " "
  // << t.passname() << " " << t.type() << std::endl;

  if (!event.exists(quad_coll_name_, in_proc_)) {
    // std::cout << "missing collection! : " << quad_coll_name_ << " " <<
    // in_proc_
    // << std::endl;
    return;
  }

  // auto
  // oneEndedQuads{event.getObject<ldmx::CaloTrigPrimCollection>(quad_coll_name_)};
  const std::vector<ldmx::CaloTrigPrim> one_ended_quads =
      event.getCollection<ldmx::CaloTrigPrim>(quad_coll_name_, in_proc_);

  //
  // sum bar ends to produce the combined quads
  std::map<int, ldmx::CaloTrigPrim> two_ended_quad_map;
  for (const auto& one_ended_quad : one_ended_quads) {
    const ldmx::HcalTriggerID end_id(one_ended_quad.getId());
    ldmx::HcalTriggerID combo_id(end_id.section(), end_id.layer(),
                                 end_id.superstrip(), 2);
    auto ptr = two_ended_quad_map.find(combo_id.raw());
    if (ptr == two_ended_quad_map.end()) {
      two_ended_quad_map[combo_id.raw()] = one_ended_quad;
    } else {
      ptr->second.setPrimitive(ptr->second.getPrimitive() +
                               one_ended_quad.getPrimitive());
    }
  }
  ldmx::CaloTrigPrimCollection two_ended_quads;
  for (auto p : two_ended_quad_map) two_ended_quads.push_back(p.second);
  event.add(combined_quad_coll_name_, two_ended_quads);

  //
  // Produce the layer-by-layer energy sums
  const unsigned int layer_max = 50;
  const unsigned int side_layer_max = 16;
  trigger::TrigEnergySumCollection back_layer_sums;
  trigger::TrigEnergySumCollection side_layer_sums;
  back_layer_sums.resize(layer_max);
  for (int i = 0; i < layer_max; i++) back_layer_sums[i].setLayer(i);
  side_layer_sums.resize(side_layer_max);
  for (int i = 0; i < side_layer_max; i++) side_layer_sums[i].setLayer(i);

  int total_adc = 0;
  std::map<int, int> section_sum;
  for (auto p : two_ended_quad_map) {
    auto tp = p.second;
    int adc = tp.getPrimitive();
    total_adc += adc;
    ldmx::HcalTriggerID combo_id(tp.getId());
    int ilayer = combo_id.layer();
    if (ilayer >= back_layer_sums.size()) {
      std::cout << "[TrigHcalEnergySum.cxx] Warning(!), layer " << ilayer
                << " is out-of-bounds.\n";
      continue;
    }
    int isec = combo_id.section();

    if (isec == 0)
      back_layer_sums[ilayer].setHwEnergy(adc +
                                          back_layer_sums[ilayer].hwEnergy());
    else
      side_layer_sums[ilayer].setHwEnergy(adc +
                                          side_layer_sums[ilayer].hwEnergy());

    auto ptr = section_sum.find(isec);
    if (ptr == section_sum.end()) {
      section_sum[isec] = adc;
    } else {
      section_sum[isec] += adc;
    }
  }
  event.add(combined_quad_coll_name_ + "BackLayerSums", back_layer_sums);
  event.add(combined_quad_coll_name_ + "SideLayerSums", side_layer_sums);

  trigger::TrigEnergySumCollection section_sums;
  for (auto p : section_sum) {
    section_sums.emplace_back(-1, p.first, p.second);
  }
  event.add(combined_quad_coll_name_ + "SectionSums", section_sums);

  // Also store total energy for now
  trigger::TrigEnergySum total_sum;
  total_sum.setLayer(-1);
  total_sum.setHwEnergy(total_adc);
  event.add(combined_quad_coll_name_ + "Sum", total_sum);
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::TrigHcalEnergySum);
