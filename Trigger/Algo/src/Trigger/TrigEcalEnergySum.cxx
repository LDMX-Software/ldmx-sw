#include "Trigger/TrigEcalEnergySum.h"

#include "../../../Algo_HLS/Ecal/src/TotalEnergy.cpp"
#include "../../../Algo_HLS/Ecal/src/data.h"
#include "DetDescr/EcalGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

namespace trigger {

void TrigEcalEnergySum::configure(framework::config::Parameters& ps) {
  //std::cout << "c++ configuring TrigEcalEnergySum" << std::endl;
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
}

void TrigEcalEnergySum::produce(framework::Event& event) {
  //std::cout << "c++ producing TrigEcalEnergySum" << std::endl;
  
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
  const ldmx::EcalGeometry& hexReadout = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  if (!event.exists(hitCollName_)) return;
  auto ecalTrigDigis{
      event.getObject<ldmx::HgcrocTrigDigiCollection>(hitCollName_)};

  // floating point algorithm
  float total_e = 0;
  // e_t total_e_trunc=0;

  // run the firmware (hls) algorithm directly
  EcalTP Input_TPs_hw[N_INPUT_TP];
  e_t energy_hw;
  int iTP = 0;
  ecalTpToE cvt;
  for (const auto& trigDigi : ecalTrigDigis) {
    // HgcrocTrigDigi

    ldmx::EcalTriggerID tid(trigDigi.getId() ); // raw value
    float e = cvt.calc(trigDigi.linearPrimitive(), tid.layer());
    // // compressed ECal digis are 8xADCs (HCal will be 4x)
    // float sie = 8 * trigDigi.linearPrimitive() * gain *
    //             mVtoMeV;  // in MeV, before layer corrections
    // float e = (sie / mipSiEnergy * layerWeights.at(tid.layer()) + sie) *
    //           secondOrderEnergyCorrection;
    total_e += e;
    // total_e_trunc = total_e_trunc + e_t(e);

    if (iTP < N_INPUT_TP) {
      Input_TPs_hw[iTP].tid = trigDigi.getId();
      Input_TPs_hw[iTP].tp = e_t(e);
    }
    iTP++;
  }

  TotalEnergy_hw(Input_TPs_hw, energy_hw);

  // std::cout << "Total ECal energy: " << total_e << " MeV (hw: " << energy_hw
  //           << " MeV)" << std::endl;
}

void TrigEcalEnergySum::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void TrigEcalEnergySum::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void TrigEcalEnergySum::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void TrigEcalEnergySum::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TrigEcalEnergySum);
