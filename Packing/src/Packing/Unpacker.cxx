
#include "Packing/Unpacker.h"

namespace packing {

void Unpacker::configure(framework::config::Parameters& ps) {
  raw_file_ = ps.getParameter<std::string>("raw_file");
  skip_unavailable_ = ps.getParameter<bool>("skip_unavailable");
}

void Unpacker::onProcessStart() {
  // open file and get tree of raw data
  reader_ = std::make_unique<Reader>(raw_file_);

  /**
   * Get the run ID and other headers
   */
}

void Unpacker::beforeNewRun(ldmx::RunHeader& header) {
  // insert run id into run header
}

void Unpacker::produce(framework::Event& event) {
  static EventPacket event_packet;

  if (!event_packet.read(reader_)) {
    AbortEvent();
  }

  /// event header information to event.getEventHeader()

  for (auto& subsys_packet : event_packet.get()) {
    /// map IDs to names and put onto event bus
    event.add(id_to_name.at(id), subsys_packet.get());
  }
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Unpacker)
