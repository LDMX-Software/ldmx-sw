
#include "Packing/SingleSubsystemPacker.h"

namespace packing {

void SingleSubsystemPacker::configure(framework::config::Parameters& ps) {
  writer_.open(ps.get<std::string>("raw_file"));
  input_name_ = ps.get<std::string>("input_name");
  input_pass_ = ps.get<std::string>("input_pass");
}

void SingleSubsystemPacker::analyze(const framework::Event& event) {
  if (!writer_) abortEvent();

  auto buff{event.getCollection<uint8_t>(input_name_, input_pass_)};
  writer_ << buff;
}

}  // namespace packing

DECLARE_PRODUCER(packing::SingleSubsystemPacker)
