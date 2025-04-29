#include "Trigger/TrigMipReco.h"

namespace trigger {

void TrigMipReco::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  passCollName_ = ps.getParameter<std::string>("passCollName");
}

void TrigMipReco::produce(framework::Event& event) {
  if (!event.exists(hitCollName_)) return;
  auto caloHits{
      event.getObject<TrigCaloHitCollection>(hitCollName_)};

  
  TrigMipCollection mips;
  std::cout << "found hits " << caloHits.size() << std::endl;
  for (const auto& tp : caloHits) {
    if( tp.section()>0 ) continue;
    // double x{0}, y{0}, z{0}; // todo
    // ldmx::HcalTriggerID combo_id(tp.getId());
    
    // int adc = tp.getPrimitive();
    // double e = adc * 1.2 / 72.961; // ADC to MeV
    // passTrigHits.emplace_back(x, y, z, e);
    
    // passTrigHits.back().setLayer(combo_id.layer());
    // passTrigHits.back().setStrip(combo_id.superstrip());
    // passTrigHits.back().setSection(combo_id.section());
  }

  //event.add(passCollName_ + "Hits", passTrigHits);
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TrigMipReco);
