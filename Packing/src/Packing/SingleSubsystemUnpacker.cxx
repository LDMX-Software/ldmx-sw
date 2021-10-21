
#include "Packing/SingleSubsystemUnpacker.h"


namespace packing {

void SingleSubsystemUnpacker::beforeNewRun(ldmx::RunHeader& rh) {
  rh.setDetectorName(detector_name_);
}

void SingleSubsystemUnpacker::configure(framework::config::Parameters& ps) {
  reader_.open(ps.getParameter<std::string>("raw_file"));
  num_words_per_event_ = ps.getParameter<int>("num_words_per_event");
  num_bytes_per_word_ = ps.getParameter<int>("num_bytes_per_word");
  output_name_ = ps.getParameter<std::string>("output_name");
  detector_name_=ps.getParameter<std::string>("detector_name");
}

void SingleSubsystemUnpacker::produce(framework::Event& event) {
  if (!reader_ or reader_.eof()) abortEvent();

  switch(num_bytes_per_word_) {
    case 1 :
      read<uint8_t>(event);
      break;
    case 2 :
      read<uint16_t>(event);
      break;
    case 4 :
      read<uint32_t>(event);
      break;
    case 8 :
      read<uint64_t>(event);
      break;
    default:
      EXCEPTION_RAISE("BadConfig",
          "Unable to handle "+std::to_string(num_bytes_per_word_)
          +" bytes per word.");
  }
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, SingleSubsystemUnpacker)
