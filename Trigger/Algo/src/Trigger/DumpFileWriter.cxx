#include "Trigger/DumpFileWriter.h"

#include "DetDescr/EcalGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

namespace trigger {

void DumpFileWriter::configure(framework::config::Parameters& ps) {
  ecal_trig_digis_passname_ = ps.get<std::string>("ecal_trig_digis_passname");
  ecal_trig_digis_event_passname_ =
      ps.get<std::string>("ecal_trig_digis_event_passname");
}

void DumpFileWriter::analyze(const framework::Event& event) {
  if (!event.exists("ecalTrigDigis", ecal_trig_digis_event_passname_)) return;
  auto ecal_trig_digis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
      "ecalTrigDigis", ecal_trig_digis_passname_)};

  // clear event to write
  my_event_.event_ = evt_no_;
  my_event_.ecal_tps_.clear();

  for (const auto& trig_digi : ecal_trig_digis) {
    // HgcrocTrigDigi

    ldmx::EcalTriggerID tid(trig_digi.getId() /*raw value*/);
    // compressed ECal digis are 8xADCs (HCal will be 4x)
    EcalTpToE cvt;
    float e = cvt.calc(trig_digi.linearPrimitive(), tid.layer());

    ldmx_int::EcalTP tp;
    // tp.fill( trigDigi.getId(), trigDigi.getPrimitive() );
    // store complete information for firmware studies
    tp.fill(trig_digi.getId(), trig_digi.getPrimitive(), tid.layer(),
            tid.module(), tid.triggercell(), int(e));
    my_event_.ecal_tps_.push_back(tp);
  }

  my_event_.writeToFile(file_);
  evt_no_++;
}

void DumpFileWriter::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  file_ = fopen(dump_file_name_.c_str(), "wb");

  return;
}

void DumpFileWriter::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  fclose(file_);

  return;
}

}  // namespace trigger

DECLARE_ANALYZER(trigger::DumpFileWriter);
