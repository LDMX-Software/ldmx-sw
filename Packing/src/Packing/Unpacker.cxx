
#include "Packing/Unpacker.h"

#include "Packing/Utility/Mask.h"
#include "Packing/RawDataFile/EventPacket.h"

namespace packing {

void Unpacker::configure(framework::config::Parameters& ps) {
  raw_file_ = ps.getParameter<std::string>("raw_file");
}

void Unpacker::onProcessStart() {
  
}

void Unpacker::beforeNewRun(ldmx::RunHeader& header) {
  // insert run id into run header
}

void Unpacker::produce(framework::Event& event) {
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Unpacker)
