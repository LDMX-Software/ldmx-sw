
#include "TrigScint/TrigScintFirmwareHitProducer.h"

#include <iterator>
#include <map>

#include "TrigScint/Firmware/hitproducer.h"
#include "TrigScint/Firmware/objdef.h"

namespace trigscint {

void TrigScintFirmwareHitProducer::configure(
    framework::config::Parameters &ps) {
  pedestal_ = ps.getParameter<double>("pedestal");
  gain_ = ps.getParameter<double>("gain");
  mevPerMip_ = ps.getParameter<double>("mev_per_mip");
  pePerMip_ = ps.getParameter<double>("pe_per_mip");
  inputCollection_ = ps.getParameter<std::string>("input_collection");
  testCollection_ = ps.getParameter<std::string>("test_collection");
  inputPassName_ = ps.getParameter<std::string>("input_pass_name");
  outputCollection_ = ps.getParameter<std::string>("output_collection");
  sample_of_interest_ = ps.getParameter<int>("sample_of_interest");
  ldmx_log(debug) << "In TrigScintFirmwareHitProducer: configure done!";
  ldmx_log(debug) << "\nPedestal: " << pedestal_ << "\nGain: " << gain_
                  << "\nMEV per MIP: " << mevPerMip_
                  << "\nPE per MIP: " << pePerMip_
                  << "\ninput collection:     " << inputCollection_
                  << "\ntest collection:	" << testCollection_
                  << "\nAre we testing:        " << doTest_
                  << "\nInput pass name:     " << inputPassName_
                  << "\nOutput collection:    " << outputCollection_;
  return;
}

void TrigScintFirmwareHitProducer::produce(framework::Event &event) {
  // This processor takes in TS QIE digis and outputs a rec hit collection. It
  // does so using hitproducer_hw, which is a validated piece of HLS code whose
  // purpose is to emulate existing reconstruction software in firmware for
  // triggering. I will more fully explain the operation and choices made in
  // hitproducer_hw in hitproducer_hw
  const auto rechits{
      event.getCollection<ldmx::TrigScintHit>(testCollection_, inputPassName_)};
  for (const auto &hit : rechits) {
    ldmx_log(debug) << "Analysis barID: " << hit.getBarID()
                    << ", PE Number: " << hit.getPE();
  }
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      inputCollection_, inputPassName_)};
  Hit out_hit[NHITS];
  ap_uint<14> fifo[NCHAN][NTIMES];
  ap_uint<8> peds[NCHAN];
  for (int i = 0; i < NCHAN; i++) {
    peds[i] = 0;
    fifo[i][0] = (peds[i] << 6) + 63;
    fifo[i][1] = (peds[i] << 6) + 63;
    fifo[i][2] = (peds[i] << 6) + 63;
    fifo[i][3] = (peds[i] << 6) + 63;
    fifo[i][4] = (peds[i] << 6) + 63;
  }
  for (const auto &digi : digis) {
    std::vector<int> adcs = digi.getADC();
    std::vector<int> tdcs = digi.getTDC();
    for (int i = 0; i < NTIMES; i++) {
      fifo[digi.getChanID()][i] = (ap_uint<14>)((adcs[i] << 6) + (tdcs[i]));
    }
  }
  hitproducer_hw(fifo, out_hit, peds);
  std::vector<ldmx::TrigScintHit> trig_scint_hits;
  for (int i = 0; i < NHITS; i++) {
    if (out_hit[i].Amp >= 3) {
      ldmx_log(debug) << "Firmware barID: " << out_hit[i].bID
                      << ", PE Number: " << out_hit[i].Amp;
      ldmx::TrigScintHit hit;
      hit.setModuleID(out_hit[i].mID);
      hit.setBarID(out_hit[i].bID);
      hit.setTime(out_hit[i].Time);
      hit.setPE(out_hit[i].Amp);
      trig_scint_hits.push_back(hit);
    }
  }
  event.add(outputCollection_, trig_scint_hits);
  return;
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintFirmwareHitProducer);
