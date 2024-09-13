#include "Trigger/DumpFileWriter.h"

#include "DetDescr/EcalGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

namespace trigger {

void DumpFileWriter::configure(framework::config::Parameters& ps) {}

void DumpFileWriter::analyze(const framework::Event& event) {
  if (!event.exists("ecalTrigDigis")) return;
  auto ecalTrigDigis{
      event.getObject<ldmx::HgcrocTrigDigiCollection>("ecalTrigDigis")};

  // clear event to write
  myEvent.event = evtNo;
  myEvent.EcalTPs.clear();

  for (const auto& trigDigi : ecalTrigDigis) {
    // HgcrocTrigDigi

    ldmx::EcalTriggerID tid(trigDigi.getId() /*raw value*/);
    // compressed ECal digis are 8xADCs (HCal will be 4x)
    ecalTpToE cvt;
    float e = cvt.calc(trigDigi.linearPrimitive(), tid.layer());
    // float sie = 8 * trigDigi.linearPrimitive() * gain *
    //             mVtoMeV;  // in MeV, before layer corrections
    // float e = (sie / mipSiEnergy * layerWeights.at(tid.layer()) + sie) *
    //           secondOrderEnergyCorrection;

    ldmx_int::EcalTP tp;
    // tp.fill( trigDigi.getId(), trigDigi.getPrimitive() );
    tp.fill(trigDigi.getId(), e);  // store linearized E
    myEvent.EcalTPs.push_back(tp);
  }

  myEvent.writeToFile(file);
  evtNo++;
}

void DumpFileWriter::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  file = fopen(dumpFileName.c_str(), "wb");

  return;
}

void DumpFileWriter::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  fclose(file);

  return;
}

}  // namespace trigger

DECLARE_ANALYZER_NS(trigger, DumpFileWriter);
