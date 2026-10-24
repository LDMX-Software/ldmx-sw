#include "Packing/ECONDDecoder.h"

#include <cstring>

#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/HcalElectronicsID.h"
#include "Ecal/EcalDetectorMap.h"
#include "Hcal/HcalDetectorMap.h"
#include "Recon/Event/ECONDEventInfo.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace packing {

namespace {

const std::array<const char*, MultiSampleECONDEventPacket::N_CORRUPTION>
    FRAME_FLAG_NAMES = {"resync (missing capture header)",
                        "lost sync",
                        "no trailer",
                        "trailer sample count mismatch",
                        "ECON ID mismatch",
                        "sample index mismatch",
                        "truncated frame",
                        "non-zero words after trailer",
                        "no sample of interest"};

const std::array<const char*, ECONDEventPacket::N_CORRUPTION>
    PACKET_FLAG_NAMES = {"bad header marker (missing ECON-D header)",
                         "bad length",
                         "bad header CRC-8",
                         "bad CRC-32",
                         "bad payload (link sub-packets do not fit)",
                         "truncated (T flag)"};

/// pack an array of flags into an int bit mask
template <std::size_t N>
int toMask(const std::array<bool, N>& flags) {
  int mask{0};
  for (std::size_t i{0}; i < N; i++) {
    if (flags[i]) mask |= (1 << i);
  }
  return mask;
}

}  // namespace

void ECONDDecoder::configure(framework::config::Parameters& ps) {
  raw_data_name_ = ps.get<std::string>("raw_data_name");
  raw_data_pass_ = ps.get<std::string>("raw_data_pass");
  digi_output_name_ = ps.get<std::string>("digi_output_name");
  info_output_name_ = ps.get<std::string>("info_output_name");
  is_ecal_ = ps.get<bool>("is_ecal");
  n_links_ = ps.get<int>("n_links");
  fiber_ = ps.get<int>("fiber");
  translate_eid_ = ps.get<bool>("translate_eid");
  roc_version_ = ps.get<int>("roc_version");
  packet_ = std::make_unique<MultiSampleECONDEventPacket>(n_links_);
}

unsigned int ECONDDecoder::channelID(int i_link, int i_ch) const {
  if (is_ecal_) {
    return ldmx::EcalElectronicsID(fiber_, i_link, i_ch).raw();
  }
  return ldmx::HcalElectronicsID(fiber_, i_link, i_ch).raw();
}

void ECONDDecoder::produce(framework::Event& event) {
  const auto& raw_binary{
      event.getCollection<uint8_t>(raw_data_name_, raw_data_pass_)};

  n_frames_++;
  ldmx::ECONDEventInfo info;
  ldmx::HgcrocDigiCollection digis;
  digis.setVersion(roc_version_);
  digis.setNumSamplesPerDigi(0);
  digis.setSampleOfInterestIndex(0);

  int frame_corruption{0};
  if (raw_binary.size() % 4 != 0) {
    ldmx_log(debug) << raw_data_name_ << " has " << raw_binary.size()
                    << " bytes which is not a multiple of 4, ignoring the "
                    << raw_binary.size() % 4 << " trailing bytes";
    frame_corruption |= (1 << ldmx::ECONDEventInfo::BAD_FRAME_SIZE);
    n_bad_frame_size_++;
  }
  // copy into 32-bit words, the data is little endian
  std::vector<uint32_t> words(raw_binary.size() / 4);
  std::memcpy(words.data(), raw_binary.data(), words.size() * 4);

  packet_->from(words);
  const auto& pkt{*packet_};
  frame_corruption |= toMask(pkt.corruption_);
  for (std::size_t i{0}; i < pkt.corruption_.size(); i++) {
    if (pkt.corruption_[i]) n_frame_flags_[i]++;
  }

  bool all_good = (frame_corruption == 0) and not pkt.samples_.empty();
  for (std::size_t i_sample{0}; i_sample < pkt.samples_.size(); i_sample++) {
    const auto& header{pkt.headers_[i_sample]};
    const auto& sample{pkt.samples_[i_sample]};
    n_packets_++;

    ldmx::ECONDSampleInfo sample_info;
    sample_info.setCaptureHeader(header.econd_id_, header.i_l1a_,
                                 header.is_soi_, header.econd_len_);
    int flags = (sample.passthrough_ ? 1 : 0) | (sample.expected_ ? 2 : 0) |
                (sample.match_ ? 4 : 0) | (sample.truncated_ ? 8 : 0) |
                (sample.subpacket_error_ ? 16 : 0);
    sample_info.setHeader(sample.payload_length_, flags, sample.ht_,
                          sample.ebo_, sample.bx_, sample.l1a_, sample.orbit_,
                          sample.reset_request_);
    sample_info.setCorruption(toMask(sample.corruption_));

    for (std::size_t i{0}; i < sample.corruption_.size(); i++) {
      if (sample.corruption_[i]) n_packet_flags_[i]++;
    }

    if (sample.isCorrupt()) {
      n_packets_corrupt_++;
      all_good = false;
    } else if (sample.isEmpty()) {
      n_packets_empty_++;
      all_good = false;
    } else {
      n_packets_decoded_++;
      for (const auto& link : sample.links_) {
        if (link.badStat()) n_links_bad_stat_++;
        ldmx::ECONDLinkInfo link_info;
        link_info.set(link.stat_, link.is_empty_, link.adc_cm0_, link.adc_cm1_,
                      link.calib_);
        sample_info.addLink(link_info);
      }
    }

    // L1A numbers are 6 bits
    if (i_sample > 0 and not sample.isCorrupt() and
        not pkt.samples_[i_sample - 1].isCorrupt() and
        sample.l1a_ != ((pkt.samples_[i_sample - 1].l1a_ + 1) & 0x3f)) {
      n_l1a_jumps_++;
    }

    info.addSample(sample_info);
  }

  info.setFrame(frame_corruption, pkt.trailer_n_samples_, pkt.i_soi_,
                pkt.n_skipped_words_);

  if (all_good) {
    std::size_t n_samples{pkt.samples_.size()};
    digis.setNumSamplesPerDigi(n_samples);
    digis.setSampleOfInterestIndex(pkt.i_soi_);
    // resort from (sample, link, channel) to (link, channel, sample)
    std::vector<uint32_t> digi(n_samples);
    for (int i_link{0}; i_link < n_links_; i_link++) {
      for (int i_ch{0}; i_ch < 36; i_ch++) {
        for (std::size_t i_sample{0}; i_sample < n_samples; i_sample++) {
          digi[i_sample] =
              pkt.samples_[i_sample].links_[i_link].channels_[i_ch];
        }
        unsigned int id{channelID(i_link, i_ch)};
        if (translate_eid_) {
          if (is_ecal_) {
            const auto& detmap{getCondition<ecal::EcalDetectorMap>(
                ecal::EcalDetectorMap::CONDITIONS_OBJECT_NAME)};
            ldmx::EcalElectronicsID eid(id);
            if (not detmap.exists(eid)) continue;
            id = detmap.get(eid).raw();
          } else {
            const auto& detmap{getCondition<hcal::HcalDetectorMap>(
                hcal::HcalDetectorMap::CONDITIONS_OBJECT_NAME)};
            ldmx::HcalElectronicsID eid(id);
            if (not detmap.exists(eid)) continue;
            id = detmap.get(eid).raw();
          }
        }
        digis.addDigi(id, digi);
      }
    }
    n_frames_filled_++;
  }
  info.setDigisFilled(all_good);

  event.add(digi_output_name_, digis);
  event.add(info_output_name_, info);
}

void ECONDDecoder::onProcessEnd() {
  ldmx_log(info) << "ECON-D decoding summary for " << raw_data_name_;
  ldmx_log(info) << "  frames seen: " << n_frames_
                 << ", digis filled: " << n_frames_filled_;
  if (n_bad_frame_size_ > 0) {
    ldmx_log(info) << "  frames with size not a multiple of 4 bytes: "
                   << n_bad_frame_size_;
  }
  for (std::size_t i{0}; i < n_frame_flags_.size(); i++) {
    if (n_frame_flags_[i] > 0) {
      ldmx_log(info) << "  frames with " << FRAME_FLAG_NAMES[i] << ": "
                     << n_frame_flags_[i];
    }
  }
  ldmx_log(info) << "  packets seen: " << n_packets_
                 << ", decoded: " << n_packets_decoded_
                 << ", empty: " << n_packets_empty_
                 << ", corrupt (skipped): " << n_packets_corrupt_;
  for (std::size_t i{0}; i < n_packet_flags_.size(); i++) {
    if (n_packet_flags_[i] > 0) {
      ldmx_log(info) << "  packets with " << PACKET_FLAG_NAMES[i] << ": "
                     << n_packet_flags_[i];
    }
  }
  if (n_links_bad_stat_ > 0) {
    ldmx_log(info) << "  decoded links with bad Stat bits: "
                   << n_links_bad_stat_;
  }
  ldmx_log(info) << "  L1A jumps between samples: " << n_l1a_jumps_;
}

}  // namespace packing

DECLARE_PRODUCER(packing::ECONDDecoder);
