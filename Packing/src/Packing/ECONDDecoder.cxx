#include "Packing/ECONDDecoder.h"

namespace packing {

void inspect(std::span<const uint32_t> data) {
  for (const auto& w: data) {
    printf("%08x\n", w);
  }
}

void ECONDDecoder::configure(framework::config::Parameters& ps) {
  raw_data_name_ = ps.get<std::string>("raw_data_name");
  raw_data_pass_ = ps.get<std::string>("raw_data_pass");
  digi_output_name_ = ps.get<std::string>("digi_output_name");
  is_ecal_ = ps.get<bool>("is_ecal");
}

void ECONDDecoder::produce(framework::Event& event) {
  const auto& raw_binary{event.getObject<std::vector<uint8_t>>(raw_data_name_, raw_data_pass_)};
  if (raw_binary.size() % 4 != 0) {
    ldmx_log(error) << raw_data_name_ << " raw binary data from pass " << raw_data_pass_
      << " contains a number of bytes that is not a multiple of 4.";
    ldmx_log(error) << "This decoding assumes 32-bit words, so the total buffer must be a multiple of 4.";
    ldmx_log(error) << "skipping this event...";
    return;
  }
  // it just so happens that we can simply re-interpret the same bytes as our 32-bit words
  // and the ordering is correct
  // this avoids an extra copy on each event which is nice
  const auto& words{*std::bit_cast<const std::vector<uint32_t>*>(&raw_binary)};

  inspect(std::span(words.begin(), words.end()-1));
}

}

DECLARE_PRODUCER(packing::ECONDDecoder);
