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

  
  TrigCaloHitCollection sortedHits;
  int evenMatrix[24][5]={};
  int evenStart[5]={99,99,99,99,99};
  int evenEnd[5]={};
  int evenCounts[5]={};
  int oddMatrix[24][5]={};
  int oddStart[5]={99,99,99,99,99};
  int oddEnd[5]={};
  int oddCounts[5]={};
  for (const auto& tp : caloHits) {
    if( tp.section()>0 || tp.energy()<minEnergy_ || tp.layer()>47) continue;
    sortedHits.push_back(tp);
    if(tp.layer()%2){
      oddMatrix[tp.layer()/2][tp.strip()] = 1;
      if( tp.layer() < oddStart[tp.strip()] ) oddStart[tp.strip()] = tp.layer();
      if( tp.layer() > oddEnd[tp.strip()] ) oddEnd[tp.strip()] = tp.layer();
    } else {
      evenMatrix[tp.layer()/2][tp.strip()] = 1;
      if( tp.layer() < evenStart[tp.strip()] ) evenStart[tp.strip()] = tp.layer();
      if( tp.layer() > evenEnd[tp.strip()] ) evenEnd[tp.strip()] = tp.layer();
    }
  }
  for(int i=0; i<24; i++){
    for(int j=0; j<5; j++){
      if(evenMatrix[i][j]) {evenCounts[j]++;}
      if(oddMatrix[i][j]) {oddCounts[j]++;}
    }
  }
  
  // straight MIP reco
  TrigMipCollection mips;
  for(int i=0; i<5; i++){
    if(oddStart[i]<80){ // start in first 5 layers
      TrigMip m;
      m.setStartLayer(oddStart[i]);
      m.setEndLayer(oddEnd[i]);
      m.setNHits(oddCounts[i]);
      m.setLength(oddEnd[i] - oddStart[i] + 1);
      m.setNHoles(m.length()/2 - m.nHits());
      mips.push_back(m);
    }
    if(evenStart[i]<80){ // start in first 5 layers
      TrigMip m;
      m.setStartLayer(evenStart[i]);
      m.setEndLayer(evenEnd[i]);
      m.setNHits(evenCounts[i]);
      m.setLength(evenEnd[i] - evenStart[i] + 1);
      m.setNHoles(m.length()/2 - m.nHits());
      mips.push_back(m);
    }
  }
  std::sort(mips.begin(), mips.end());

  event.add(passCollName_, mips);



  // std::sort(sortedHits.begin(), sortedHits.end(),
  // 	    [](TrigCaloHit a, TrigCaloHit b) {
  // 	      return a.layer() > b.layer();
  // 	    });
  
  // run inside-out tracking
  //std::cout << "new"() << std::endl;
  // for (const auto& tp : caloHits) {
  //   std::cout << tp.layer() << std::endl;
    
  // }
  
    // double x{0}, y{0}, z{0}; // todo
    // ldmx::HcalTriggerID combo_id(tp.getId());
    
    // int adc = tp.getPrimitive();
    // double e = adc * 1.2 / 72.961; // ADC to MeV
    // passTrigHits.emplace_back(x, y, z, e);
    
    // passTrigHits.back().setLayer(combo_id.layer());
    // passTrigHits.back().setStrip(combo_id.superstrip());
    // passTrigHits.back().setSection(combo_id.section());
  

  //event.add(passCollName_ + "Hits", passTrigHits);
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TrigMipReco);
