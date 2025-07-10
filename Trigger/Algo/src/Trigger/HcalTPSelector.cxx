#include "Trigger/HcalTPSelector.h"

namespace trigger {

void HcalTPSelector::configure(framework::config::Parameters& ps) {
  // ideally, we would directly grab the TPs here instead of
  // building them in the HCal energy sum collection
  combinedQuadCollName_ = ps.getParameter<std::string>("combinedQuadCollName");
  passCollName_ = ps.getParameter<std::string>("passCollName");
}

void HcalTPSelector::produce(framework::Event& event) {
  if (!event.exists(combinedQuadCollName_, passCollName_)) return;
  auto hcalTPs{
      event.getObject<ldmx::CaloTrigPrimCollection>(combinedQuadCollName_, passCollName_)};

  // Should move the TP building itself here
  // In the meantime, create the "analysis object" hits

  TrigCaloHitCollection passTrigHits;
  for (const auto& tp : hcalTPs) {
    double x{0}, y{0}, z{0};  // todo
    ldmx::HcalTriggerID combo_id(tp.getId());

    int adc = tp.getPrimitive();
    double e = adc * 1.2 / 72.961;  // ADC to MeV
    passTrigHits.emplace_back(x, y, z, e);

    passTrigHits.back().setLayer(combo_id.layer());
    passTrigHits.back().setStrip(combo_id.superstrip());
    passTrigHits.back().setSection(combo_id.section());
  }

  event.add(passCollName_ + "Hits", passTrigHits);
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::HcalTPSelector);
