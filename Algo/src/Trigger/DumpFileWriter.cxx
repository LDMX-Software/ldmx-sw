#include "Trigger/DumpFileWriter.h"

#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "DetDescr/EcalHexReadout.h"

namespace ldmx {

void DumpFileWriter::configure(Parameters &ps) {
      
}

void DumpFileWriter::analyze(const Event &event) {

  const EcalTriggerGeometry& geom=getCondition<EcalTriggerGeometry>(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
  const EcalHexReadout& hexReadout = getCondition<EcalHexReadout>(EcalHexReadout::CONDITIONS_OBJECT_NAME);
	
  if (!event.exists("ecalTrigDigis")) return;
  auto ecalTrigDigis{event.getObject<HgcrocTrigDigiCollection>("ecalTrigDigis")};
	
  // clear event to write
  myEvent.event = evtNo;
  myEvent.EcalTPs.clear();

  for(const auto& trigDigi : ecalTrigDigis){
    // HgcrocTrigDigi

    EcalTriggerID tid( trigDigi.getId() /*raw value*/ );
    // compressed ECal digis are 8xADCs (HCal will be 4x)
    float sie =  8*trigDigi.linearPrimitive() * gain * mVtoMeV; // in MeV, before layer corrections
    float e = (sie/mipSiEnergy * layerWeights.at( tid.layer() ) + sie) * secondOrderEnergyCorrection;
	    
    ldmx_int::EcalTP tp;
    //tp.fill( trigDigi.getId(), trigDigi.getPrimitive() );
    tp.fill( trigDigi.getId(), e ); // store linearized E
    myEvent.EcalTPs.push_back(tp);
  }

  myEvent.writeToFile(file);
  evtNo++;
}


void DumpFileWriter::onFileOpen() {

  ldmx_log(debug) << "Opening file!";

  return;
}

void DumpFileWriter::onFileClose() {

  ldmx_log(debug) << "Closing file!";

  return;
}

void DumpFileWriter::onProcessStart() {

  ldmx_log(debug) << "Process starts!";

  file = fopen(dumpFileName.c_str(),"wb");

  return;
}

void DumpFileWriter::onProcessEnd() {

  ldmx_log(debug) << "Process ends!";

  fclose(file);

  return;
}

}

DECLARE_ANALYZER_NS(ldmx, DumpFileWriter);
