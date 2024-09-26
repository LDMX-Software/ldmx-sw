
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
  verbose_ = ps.getParameter<bool>("verbose");
  sample_of_interest_ = ps.getParameter<int>("sample_of_interest");
  if (verbose_) {
    ldmx_log(info) << "In TrigScintFirmwareHitProducer: configure done!";
    ldmx_log(info) << "\nPedestal: " << pedestal_ << "\nGain: " << gain_
                   << "\nMEV per MIP: " << mevPerMip_
                   << "\nPE per MIP: " << pePerMip_
                   << "\ninput collection:     " << inputCollection_
                   << "\ntest collection:	" << testCollection_
                   << "\nAre we testing:        " << doTest_
                   << "\nInput pass name:     " << inputPassName_
                   << "\nOutput collection:    " << outputCollection_
                   << "\nVerbosity: " << verbose_;
  }

  return;
}

void TrigScintFirmwareHitProducer::produce(framework::Event &event) {
  // This processor takes in TS QIE digis and outputs a rec hit collection. It
  // does so using hitproducer_hw, which is a validated piece of HLS code whose
  // purpose is to emulate existing reconstruction software in firmware for
  // triggering. I will more fully explain the operation and choices made in
  // hitproducer_hw in hitproducer_hw
  if (verbose_) {
    const auto rechits{event.getCollection<ldmx::TrigScintHit>(testCollection_,
                                                               inputPassName_)};
    for (const auto &hit : rechits) {
      ldmx_log(debug) << "Analysis barID: " << hit.getBarID()
                << ", PE Number: " << hit.getPE();
    }
  }
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      inputCollection_, inputPassName_)};
  Hit outHit[NHITS];
  ap_uint<14> FIFO[NCHAN][NTIMES];
  ap_uint<8> Peds[NCHAN];
  for (int i = 0; i < NCHAN; i++) {
    Peds[i] = 0;
    FIFO[i][0] = (Peds[i] << 6) + 63;
    FIFO[i][1] = (Peds[i] << 6) + 63;
    FIFO[i][2] = (Peds[i] << 6) + 63;
    FIFO[i][3] = (Peds[i] << 6) + 63;
    FIFO[i][4] = (Peds[i] << 6) + 63;
  }
  for (const auto &digi : digis) {
    std::vector<int> adcs = digi.getADC();
    std::vector<int> tdcs = digi.getTDC();
    for (int i = 0; i < NTIMES; i++) {
      FIFO[digi.getChanID()][i] = (ap_uint<14>)((adcs[i] << 6) + (tdcs[i]));
    }
  }
  hitproducer_hw(FIFO, outHit, Peds);
  std::vector<ldmx::TrigScintHit> trigScintHits;
  for (int i = 0; i < NHITS; i++) {
    if (outHit[i].Amp >= 3) {
      if (verbose_) {
        ldmx_log(debug) << "Firmware barID: " << outHit[i].bID
                  << ", PE Number: " << outHit[i].Amp;
      }
      ldmx::TrigScintHit hit;
      hit.setModuleID(outHit[i].mID);
      hit.setBarID(outHit[i].bID);
      hit.setTime(outHit[i].Time);
      hit.setPE(outHit[i].Amp);
      trigScintHits.push_back(hit);
    }
  }
  event.add(outputCollection_, trigScintHits);
  return;
}

}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TrigScintFirmwareHitProducer);
