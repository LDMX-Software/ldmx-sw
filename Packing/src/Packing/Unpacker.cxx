
#include "Packing/Unpacker.h"

#include "Packing/Utility/Mask.h"
#include "Packing/RawDataFile/EventPacket.h"

namespace packing {

void Unpacker::configure(framework::config::Parameters& ps) {
  raw_file_ = ps.getParameter<std::string>("raw_file");
  skip_unavailable_ = ps.getParameter<bool>("skip_unavailable");
}

void Unpacker::onProcessStart() {
  // open file and get tree of raw data
  reader_.open(raw_file_);

  /**
   * Get the run ID and other headers
   */
  uint32_t word;
  reader >> word;

  uint8_t version = word >> 28;
  if (version != 0) {
    EXCEPTION_RAISE("RawFileVersion",
        "Unable to handle raw file version "
        +std::to_string(version));
  }

  run_ = word & utility::mask<28>;
}

void Unpacker::beforeNewRun(ldmx::RunHeader& header) {
  // insert run id into run header
}

void Unpacker::produce(framework::Event& event) {
  static rawdatafile::EventPacket event_packet;

  reader_ >> event_packet;
  if (!reader_) {
    /** ERROR or EOF */
  }

  /*
  /// event header information to event.getEventHeader()

  for (auto& subsys_packet : event_packet.get()) {
    /// map IDs to names and put onto event bus
    event.add("id_to_name.at(id)", subsys_packet.get());
  }
  */
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Unpacker)
