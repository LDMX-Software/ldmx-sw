#include "Tracking/Reco/RawTrackerDecoder.h"

#include <cstring>
#include <vector>

#include "Tracking/Event/RawSiStripHit.h"

namespace tracking::reco {

void RawTrackerDecoder::configure(framework::config::Parameters& ps) {
  input_collection_ = ps.get<std::string>("input_collection", input_collection_);
  input_pass_name_ = ps.get<std::string>("input_pass_name", input_pass_name_);
  output_collection_ = ps.get<std::string>("output_collection", output_collection_);
  n_samples_ = ps.get<int>("n_samples", n_samples_);
}

void RawTrackerDecoder::produce(framework::Event& event) {
  const auto& raw =
      event.getCollection<uint8_t>(input_collection_, input_pass_name_);

  std::vector<ldmx::RawSiStripHit> hits;

  // Need at least the 10-byte header (4B event_count + 6B padding).
  if (raw.size() < 10) {
    event.add(output_collection_, hits);
    return;
  }

  uint32_t event_count{0};
  std::memcpy(&event_count, raw.data(), sizeof(event_count));

  const std::size_t payload_start = 10;
  const std::size_t n_multisamples = (raw.size() - payload_start) / 10;

  if (n_multisamples == 0) {
    event.add(output_collection_, hits);
    return;
  }

  const auto& hdr = event.getEventHeader();
  uint64_t ror_ts = 0;
  if (hdr.hasIntParameter("RoR Timestamp LSB") &&
      hdr.hasIntParameter("RoR Timestamp MSB")) {
    ror_ts = (static_cast<uint64_t>(
                  static_cast<uint32_t>(hdr.getIntParameter("RoR Timestamp MSB"))) << 32) |
             static_cast<uint32_t>(hdr.getIntParameter("RoR Timestamp LSB"));
  }
  auto timestamp = static_cast<long>(ror_ts);

  hits.reserve(n_multisamples - 1);

  // Skip the last multisample (replicates FrameParser behaviour).
  for (std::size_t i = 0; i < n_multisamples - 1; ++i) {
    const uint8_t* ms = raw.data() + payload_start + i * 10;

    uint16_t s0{0}, s1{0}, s2{0};
    std::memcpy(&s0, ms + 0, 2);
    std::memcpy(&s1, ms + 2, 2);
    std::memcpy(&s2, ms + 4, 2);

    uint8_t channel   = ms[6];
    uint8_t apv_id    = ms[7] & 0x07;
    uint8_t hybrid_id = (ms[7] >> 4) & 0x07;
    uint8_t feb_id    = ms[8] & 0x0F;

    // Bytes 8–9 as little-endian uint16_t; shift right 4, mask 6 bits.
    uint16_t trig_word{0};
    std::memcpy(&trig_word, ms + 8, 2);
    uint16_t apv_trigger = (trig_word >> 4) & 0x3F;

    uint8_t flags      = ms[9];
    uint8_t read_error = (flags >> 2) & 1;
    uint8_t tail       = (flags >> 3) & 1;
    uint8_t head       = (flags >> 4) & 1;
    uint8_t filter     = (flags >> 5) & 1;

    ldmx::RawSiStripHit hit(channel,
                            std::vector<short>{static_cast<short>(s0),
                                               static_cast<short>(s1),
                                               static_cast<short>(s2)},
                            timestamp);
    hit.setApvId(apv_id);
    hit.setHybridId(hybrid_id);
    hit.setFebId(feb_id);
    hit.setApvTrigger(apv_trigger);
    hit.setReadError(read_error);
    hit.setHead(head);
    hit.setTail(tail);
    hit.setFilter(filter);
    hits.push_back(std::move(hit));
  }

  event.add(output_collection_, hits);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::RawTrackerDecoder)
