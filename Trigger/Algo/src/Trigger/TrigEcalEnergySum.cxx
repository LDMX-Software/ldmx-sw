#include "Trigger/TrigEcalEnergySum.h"

#include "../../../Algo_HLS/Ecal/src/TotalEnergy.cpp"
#include "../../../Algo_HLS/Ecal/src/data.h"
#include "DetDescr/EcalGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

namespace trigger {

void TrigEcalEnergySum::configure(framework::config::Parameters& ps) {
  hit_coll_name_ = ps.get<std::string>("hitCollName");
  hit_coll_passname_ = ps.get<std::string>("hit_coll_passname");
  hit_collname_events_passname_ =
      ps.get<std::string>("hit_collname_events_passname");
}

void TrigEcalEnergySum::produce(framework::Event& event) {
  if (!event.exists(hit_coll_name_, hit_collname_events_passname_)) return;
  auto ecal_trig_digis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
      hit_coll_name_, hit_coll_passname_)};

  // floating point algorithm
  // float total_e = 0;
  // e_t total_e_trunc=0;

  // run the firmware (hls) algorithm directly
  EcalTP input_t_ps_hw[N_INPUT_TP];
  e_t energy_hw;
  int i_tp = 0;
  EcalTpToE cvt;
  for (const auto& trig_digi : ecal_trig_digis) {
    // HgcrocTrigDigi

    ldmx::EcalTriggerID tid(trig_digi.getId());  // raw value
    float e = cvt.calc(trig_digi.linearPrimitive(), tid.layer());
    // // compressed ECal digis are 8xADCs (HCal will be 4x)
    // float sie = 8 * trigDigi.linearPrimitive() * gain *
    //             mVtoMeV;  // in MeV, before layer corrections
    // float e = (sie / mipSiEnergy * layerWeights.at(tid.layer()) + sie) *
    //           secondOrderEnergyCorrection;
    // total_e += e;
    // total_e_trunc = total_e_trunc + e_t(e);

    if (i_tp < N_INPUT_TP) {
      input_t_ps_hw[i_tp].tid_ = trig_digi.getId();
      input_t_ps_hw[i_tp].tp_ = e_t(e);
    }
    i_tp++;
  }

  TotalEnergy_hw(input_t_ps_hw, energy_hw);

  // std::cout << "Total ECal energy: " << total_e << " MeV (hw: " << energy_hw
  //           << " MeV)" << std::endl;
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::TrigEcalEnergySum);
