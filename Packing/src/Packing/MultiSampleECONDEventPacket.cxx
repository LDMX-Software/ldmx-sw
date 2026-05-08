#include "Packing/MultiSampleECONDEventPacket.h"

#include "Packing/DAQSampleHeader.h"
#include "Packing/Utility/Hex.h"
using packing::utility::Hex;

namespace packing {

MultiSampleECONDEventPacket::MultiSampleECONDEventPacket(int n_links)
    : n_links_{n_links} {}

const std::string MultiSampleECONDEventPacket::to_csv_header =
    "timestamp,orbit,bx,event,i_link,channel,i_sample,Tp,Tc,adc_tm1,adc,tot,"
    "toa";

void MultiSampleECONDEventPacket::to_csv(std::ofstream& f) const {
  /**
   * The columns of the output CSV are
   * ```
   * timestamp,orbit,bx,event,i_link,channel,i_sample,Tp,Tc,adc_tm1,adc,tot,toa
   * ```
   */
  for (std::size_t i_sample{0}; i_sample < samples.size(); i_sample++) {
    const auto& sample{samples[i_sample]};
    for (std::size_t i_link{0}; i_link < sample.links.size(); i_link++) {
      const auto& daq_link{sample.links[i_link]};
      f << timestamp << ',' << daq_link.orbit << ',' << daq_link.bx << ','
        << daq_link.event << ',' << i_link << ',' << "calib," << i_sample
        << ',';
      daq_link.calib.to_csv(f);
      f << '\n';
      for (std::size_t i_ch{0}; i_ch < 36; i_ch++) {
        f << timestamp << ',' << daq_link.orbit << ',' << daq_link.bx << ','
          << daq_link.event << ',' << i_link << ',' << i_ch << ',' << i_sample
          << ',';
        daq_link.channels[i_ch].to_csv(f);
        f << '\n';
      }
    }
  }
}

void MultiSampleECONDEventPacket::from(std::span<uint32_t> frame,
                                       bool expect_ldmx_ror_header) {
  samples.clear();
  contrib_id = 0;
  subsys_id = 0;
  timestamp = 0;

  std::size_t offset{0};
  if (expect_ldmx_ror_header) {
    /**
     * the ldmx ror header inserts 4 32-bit words (16 bytes) of the form
     *
     * 0xA5 | 8b contrib | 8b subsys | 8b VERS = 0
     * 32b 0
     * 64b timestamp
     */
    uint8_t sentinel = static_cast<uint8_t>((frame[0] >> 24) & 0xff);
    contrib_id = ((frame[0] >> 16) & 0xff);
    subsys_id = ((frame[0] >> 8) & 0xff);
    uint8_t vers = static_cast<uint8_t>(frame[0] & 0xff);

    // frame[1] == 0
    // timestamp is in frame[2] and frame[3]
    // combine into uint64_t because timestamp is 64b but each word is 32b
    timestamp = (static_cast<uint64_t>(frame[3]) << 32) |
                static_cast<uint64_t>(frame[2]);
    offset += 4;
  }
  std::size_t i_sample{0};
  DAQSampleHeader header;
  while (offset < frame.size()) {
    header.from(frame[offset]);
    if (header.is_ending_trailer()) {
      pflib_log(trace) << "Last sample packet found, leaving loop at offset = "
                       << offset << " (frame.size() = " << frame.size() << ")";
      break;
    }

    if (i_sample > 0 and econd_id != header.econd_id) {
      pflib_log(warn) << "ECON ID mismatch: Found " << header.econd_id
                      << " but this stream was " << econd_id << " earlier";
    }
    econd_id = header.econd_id;
    pflib_log(trace) << Hex(frame[offset]) << " -> " << header;

    // header decoded, shift offset
    offset++;

    if (i_sample != header.i_l1a) {
      pflib_log(warn) << "mismatch between transmitted index " << header.i_l1a
                      << " and unpacking index for sample " << i_sample;
    }

    if (header.is_soi) {
      i_soi = i_sample;
    }

    samples.emplace_back(n_links_);
    samples.back().from(frame.subspan(offset, header.econd_len));
    offset += header.econd_len;

    i_sample++;
  }
}

Reader& MultiSampleECONDEventPacket::read(Reader& r) {
  /**
   * DANGER
   * Without signal header/trailer words, this assumes that the data
   * stream is word aligned and we aren't starting on the wrong word.
   */
  std::vector<uint32_t> frame;
  DAQSampleHeader header;
  while (r) {
    uint32_t word{0};
    if (!(r >> word)) {
      // is this worthy of a warning?
      pflib_log(trace)
          << "leaving frame accumulation loop failing to pop next header word";
      break;
    }
    frame.push_back(word);
    header.from(word);
    pflib_log(trace) << Hex(word) << " -> " << header;
    if (header.is_ending_trailer()) {
      pflib_log(trace) << "found special header marking end of packet"
                       << ", leaving accumulation loop";
      break;
    }
    if (!r.read(frame, header.econd_len, frame.size())) {
      pflib_log(warn) << "partially transmitted frame!";
      return r;
    }
    pflib_log(trace) << Hex(*(frame.end() - 1));
  }

  this->from(frame);
  return r;
}

const ECONDEventPacket& MultiSampleECONDEventPacket::soi() const {
  return samples.at(i_soi);
}

}  // namespace pflib::packing
