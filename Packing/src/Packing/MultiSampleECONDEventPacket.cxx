#include "Packing/MultiSampleECONDEventPacket.h"

#include <algorithm>

#include "Packing/Utility/Hex.h"

using packing::utility::Hex;

namespace packing {

MultiSampleECONDEventPacket::MultiSampleECONDEventPacket(int n_links)
    : n_links_{n_links} {}

const ECONDEventPacket& MultiSampleECONDEventPacket::soi() const {
  return samples_.at(i_soi_);
}

bool MultiSampleECONDEventPacket::isCorrupt() const {
  return std::any_of(corruption_.begin(), corruption_.end(),
                     [](bool c) { return c; });
}

std::size_t MultiSampleECONDEventPacket::resync(std::span<const uint32_t> frame,
                                                std::size_t start,
                                                std::size_t i_sample) const {
  DAQSampleHeader header;
  for (std::size_t i{start}; i < frame.size(); i++) {
    header.from(frame[i]);
    if (header.version_ == DAQSampleHeader::VERSION and
        header.isEndingTrailer()) {
      return i;
    }
    if (DAQSampleHeader::isValid(frame[i]) and header.i_l1a_ == i_sample and
        (econd_id_ < 0 or
         header.econd_id_ == static_cast<uint32_t>(econd_id_))) {
      return i;
    }
  }
  return frame.size();
}

void MultiSampleECONDEventPacket::from(std::span<const uint32_t> frame) {
  corruption_.fill(false);
  i_soi_ = 0;
  econd_id_ = -1;
  trailer_n_samples_ = -1;
  n_skipped_words_ = 0;
  headers_.clear();
  samples_.clear();

  bool found_soi{false};
  std::size_t offset{0};
  DAQSampleHeader header;
  while (offset < frame.size()) {
    header.from(frame[offset]);
    if (header.version_ == DAQSampleHeader::VERSION and
        header.isEndingTrailer()) {
      trailer_n_samples_ = DAQSampleHeader::trailerNumSamples(frame[offset]);
      ldmx_log(trace) << "Last sample packet found at offset = " << offset
                      << " (frame.size() = " << frame.size() << ")";
      offset++;
      break;
    }

    bool valid =
        DAQSampleHeader::isValid(frame[offset]) and
        (econd_id_ < 0 or header.econd_id_ == static_cast<uint32_t>(econd_id_));
    if (not valid) {
      if (DAQSampleHeader::isValid(frame[offset])) {
        corruption_[ECON_ID_MISMATCH] = true;
      }
      auto next = resync(frame, offset + 1, samples_.size());
      ldmx_log(debug) << Hex(frame[offset])
                      << " is not a capture header, skipping "
                      << (next - offset) << " words";
      corruption_[RESYNC] = true;
      n_skipped_words_ += next - offset;
      if (next == frame.size()) {
        corruption_[LOST_SYNC] = true;
      }
      offset = next;
      continue;
    }

    if (econd_id_ < 0) econd_id_ = header.econd_id_;
    ldmx_log(trace) << Hex(frame[offset]) << " -> " << header;

    // header decoded, shift offset
    offset++;

    std::size_t i_sample{samples_.size()};
    if (i_sample != header.i_l1a_) {
      ldmx_log(debug) << "mismatch between transmitted index " << header.i_l1a_
                      << " and unpacking index for sample " << i_sample;
      corruption_[SAMPLE_INDEX_MISMATCH] = true;
    }

    if (header.is_soi_ and not found_soi) {
      i_soi_ = i_sample;
      found_soi = true;
    }

    std::size_t len{header.econd_len_};
    if (offset + len > frame.size()) {
      ldmx_log(debug) << "capture header length " << len << " goes past the "
                      << "end of the frame by " << (offset + len - frame.size())
                      << " words";
      corruption_[TRUNCATED_FRAME] = true;
      len = frame.size() - offset;
    }

    headers_.push_back(header);
    samples_.emplace_back(n_links_);
    samples_.back().from(frame.subspan(offset, len));
    offset += len;
  }

  if (trailer_n_samples_ < 0) {
    corruption_[NO_TRAILER] = true;
  } else if (static_cast<std::size_t>(trailer_n_samples_) != samples_.size()) {
    ldmx_log(debug) << "trailer says " << trailer_n_samples_
                    << " samples but found " << samples_.size();
    corruption_[TRAILER_COUNT_MISMATCH] = true;
  }

  // only zero padding is expected after the trailer
  if (std::any_of(frame.begin() + offset, frame.end(),
                  [](uint32_t w) { return w != 0; })) {
    ldmx_log(debug) << "non-zero words after the ending trailer";
    corruption_[EXTRA_WORDS] = true;
  }

  if (not samples_.empty() and not found_soi) {
    corruption_[NO_SOI] = true;
  }
}

}  // namespace packing
