#include "Trigger/HcalTPSelector.h"

namespace trigger {

void HcalTPSelector::configure(framework::config::Parameters& ps) {
  // ideally, we would directly grab the TPs here instead of
  // building them in the HCal energy sum collection
  combined_quad_coll_name_ = ps.get<std::string>("combined_quad_coll_name");
  pass_coll_name_ = ps.get<std::string>("pass_coll_name");
  tp_coll_passname_ = ps.get<std::string>("tp_coll_pass_name");
  tp_coll_event_passname_ = ps.get<std::string>("tp_coll_event_passname");
}

void HcalTPSelector::produce(framework::Event& event) {
  if (!event.exists(combined_quad_coll_name_, tp_coll_event_passname_)) return;
  auto hcalTPs{event.getObject<ldmx::CaloTrigPrimCollection>(
      combined_quad_coll_name_, tp_coll_passname_)};

  // Should move the TP building itself here
  // In the meantime, create the "analysis object" hits

  std::vector<TrigCaloHit> passTrigHits;
  for (const auto& tp : hcalTPs) {
    double x{0}, y{0}, z{0};  // todo
    ldmx::HcalTriggerID combo_id(tp.getId());

    int adc = tp.getPrimitive();
    // mV/ADC: 1.2
    // MeV/MIP: 4.66
    // PE/MIP: 68 (summed over BOTH ends, based on 1808.05219, p38)
    // mV/PE: 5
    // mV/MeV: 72.961 (= 5*68/4.66)
    double energy =
        adc * 1.2 / 72.961;  // ADC to MeV based on values just above
    passTrigHits.emplace_back(x, y, z, energy);

    passTrigHits.back().setLayer(combo_id.layer());
    passTrigHits.back().setStrip(combo_id.superstrip());
    passTrigHits.back().setSection(combo_id.section());
  }

  event.add(pass_coll_name_ + "Hits", passTrigHits);
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::HcalTPSelector);
