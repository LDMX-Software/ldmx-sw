
#include "TrigScint/TrigScintFirmwareHitProducer.h"

#include <iterator>
#include <map>

#include "TrigScint/Firmware/hitproducer.h"
#include "TrigScint/Firmware/objdef.h"

namespace trigscint {

void TrigScintFirmwareHitProducer::configure(
    framework::config::Parameters &ps) {
  pedestal_ = ps.get<double>("pedestal");
  gain_ = ps.get<double>("gain");
  mev_per_mip_ = ps.get<double>("mev_per_mip");
  pe_per_mip_ = ps.get<double>("pe_per_mip");
  input_collection_ = ps.get<std::string>("input_collection");
  test_collection_ = ps.get<std::string>("test_collection");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  output_collection_ = ps.get<std::string>("output_collection");
  sample_of_interest_ = ps.get<int>("sample_of_interest");
  ldmx_log(debug) << "In TrigScintFirmwareHitProducer: configure done!";
  ldmx_log(debug) << "\nPedestal: " << pedestal_ << "\nGain: " << gain_
                  << "\nMEV per MIP: " << mev_per_mip_
                  << "\nPE per MIP: " << pe_per_mip_
                  << "\ninput collection:     " << input_collection_
                  << "\ntest collection:	" << test_collection_
                  << "\nAre we testing:        " << do_test_
                  << "\nInput pass name:     " << input_pass_name_
                  << "\nOutput collection:    " << output_collection_;
  return;
}

void TrigScintFirmwareHitProducer::produce(framework::Event &event) {
  // This processor takes in TS QIE digis and outputs a rec hit collection. It
  // does so using hitproducerHw, which is a validated piece of HLS code whose
  // purpose is to emulate existing reconstruction software in firmware for
  // triggering. I will more fully explain the operation and choices made in
  // hitproducerHw in hitproducerHw
  const auto rechits{event.getCollection<ldmx::TrigScintHit>(test_collection_,
                                                             input_pass_name_)};
  for (const auto &hit : rechits) {
    ldmx_log(debug) << "Analysis barID: " << hit.getBarID()
                    << ", PE Number: " << hit.getPE();
  }
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      input_collection_, input_pass_name_)};
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
      fifo[digi.getChanID()][i] = static_cast<ap_uint<14>>((adcs.at(i) << 6) + (tdcs.at(i)));
    }
  }
  hitproducerHw(fifo, out_hit, peds);
  std::vector<ldmx::TrigScintHit> trig_scint_hits;
  for (int i = 0; i < NHITS; i++) {
    if (out_hit[i].amp_ >= 3) {
      ldmx_log(debug) << "Firmware barID: " << out_hit[i].b_id_
                      << ", PE Number: " << out_hit[i].amp_;
      ldmx::TrigScintHit hit;
      hit.setModuleID(out_hit[i].m_id_);
      hit.setBarID(out_hit[i].b_id_);
      hit.setTime(out_hit[i].time_);
      hit.setPE(out_hit[i].amp_);
      trig_scint_hits.push_back(hit);
    }
  }
  event.add(output_collection_, trig_scint_hits);
  return;
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintFirmwareHitProducer);
