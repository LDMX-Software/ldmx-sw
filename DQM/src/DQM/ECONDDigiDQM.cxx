#include "DQM/ECONDDigiDQM.h"

#include "Recon/Event/ECONDEventInfo.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace dqm {

void ECONDDigiDQM::configure(framework::config::Parameters& ps) {
  digi_name_ = ps.get<std::string>("digi_name");
  info_name_ = ps.get<std::string>("info_name");
  input_pass_ = ps.get<std::string>("input_pass");
}

void ECONDDigiDQM::onProcessStart() {
  getHistoDirectory();
  histograms_.create("frame_flags", "",
                     {"good", "resync", "lost sync", "no trailer",
                      "trailer count", "ECON ID", "sample index", "truncated",
                      "extra words", "no SOI", "frame size", "no frame"});
  histograms_.create("packet_flags", "",
                     {"good", "empty", "header marker", "length", "header CRC",
                      "CRC-32", "payload", "truncated"});
  histograms_.create("n_samples", "Samples per Event", 33, -0.5, 32.5);
  histograms_.create("i_soi", "Index of Sample of Interest", 33, -0.5, 32.5);
  histograms_.create("bx", "BX", 128, 0, 4096);
  histograms_.create("link_bad_stat", "Link Index", 12, -0.5, 11.5);
  histograms_.create("adc_cm", "Common Mode ADC", 256, 0, 1024);
  histograms_.create("calib_adc", "Calibration Channel ADC", 256, 0, 1024);
  histograms_.create("n_digis", "Digis per Event", 100, 0, 500);
  histograms_.create("flags", "Tc Tp Flags", {"00", "01", "10", "11"});
  histograms_.create("adc", "ADC", 256, 0, 1024);
  histograms_.create("adc_tm1", "ADC(t-1)", 256, 0, 1024);
  histograms_.create("toa", "TOA", 256, 0, 1024);
  histograms_.create("tot", "TOT (10-bit compressed)", 256, 0, 1024);
  histograms_.create("soi_adc_by_channel", "Digi Index", 432, -0.5, 431.5,
                     "ADC in Sample of Interest", 128, 0, 1024);
}

void ECONDDigiDQM::analyze(const framework::Event& event) {
  const auto& info{
      event.getObject<ldmx::ECONDEventInfo>(info_name_, input_pass_)};
  const auto& digis{
      event.getObject<ldmx::HgcrocDigiCollection>(digi_name_, input_pass_)};

  // bin 0 is good, bit i of the mask is bin i+1
  int frame_mask = info.getCorruption();
  if (frame_mask == 0) histograms_.fill("frame_flags", 0);
  for (int i{0}; i < 11; i++) {
    if ((frame_mask >> i) & 1) histograms_.fill("frame_flags", i + 1);
  }
  if (info.isCorrupt(ldmx::ECONDEventInfo::NO_FRAME)) return;

  histograms_.fill("n_samples", info.getSamples().size());
  histograms_.fill("i_soi", info.getSOIIndex());
  for (const auto& sample : info.getSamples()) {
    int mask = sample.getCorruption();
    if (mask == 0) {
      histograms_.fill("packet_flags", sample.getPayloadLength() == 0 ? 1 : 0);
      histograms_.fill("bx", sample.getBX());
    }
    for (int i{0}; i < 6; i++) {
      if ((mask >> i) & 1) histograms_.fill("packet_flags", i + 2);
    }
    const auto& links{sample.getLinks()};
    for (std::size_t i_link{0}; i_link < links.size(); i_link++) {
      const auto& link{links[i_link]};
      if (link.hasBadStat()) histograms_.fill("link_bad_stat", i_link);
      histograms_.fill("adc_cm", link.getADCCM0());
      histograms_.fill("adc_cm", link.getADCCM1());
      histograms_.fill("calib_adc", (link.getCalib() >> 10) & 0x3ff);
    }
  }

  histograms_.fill("n_digis", digis.getNumDigis());
  for (unsigned int i_digi{0}; i_digi < digis.getNumDigis(); i_digi++) {
    auto digi{digis.getDigi(i_digi)};
    for (unsigned int i_sample{0}; i_sample < digi.size(); i_sample++) {
      uint32_t w = digi.at(i_sample).raw();
      bool tc = (w >> 31) & 1;
      bool tp = (w >> 30) & 1;
      histograms_.fill("flags", 2 * tc + tp);
      histograms_.fill("toa", w & 0x3ff);
      if (tc) {
        histograms_.fill("tot", (w >> 10) & 0x3ff);
      } else {
        histograms_.fill("adc", (w >> 10) & 0x3ff);
        histograms_.fill("adc_tm1", (w >> 20) & 0x3ff);
      }
    }
    uint32_t soi = digi.soi().raw();
    if (((soi >> 31) & 1) == 0) {
      histograms_.fill("soi_adc_by_channel", i_digi, (soi >> 10) & 0x3ff);
    }
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::ECONDDigiDQM)
