
#include "Packing/SingleSubsystemPacker.h"


namespace packing {

void SingleSubsystemPacker::configure(framework::config::Parameters& ps) {
  writer_.open(ps.getParameter<std::string>("raw_file"));
  num_bytes_per_word_ = ps.getParameter<int>("num_bytes_per_word");
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
}

void SingleSubsystemPacker::analyze(const framework::Event& event) {
  if (!writer_) abortEvent();

  switch(num_bytes_per_word_) {
    case 1 :
      write<uint8_t>(event);
      break;
    case 2 :
      write<uint16_t>(event);
      break;
    case 4 :
      write<uint32_t>(event);
      break;
    case 8 :
      write<uint64_t>(event);
      break;
    default:
      EXCEPTION_RAISE("BadConfig",
          "Unable to handle "+std::to_string(num_bytes_per_word_)
          +" bytes per word.");
  }
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, SingleSubsystemPacker)
