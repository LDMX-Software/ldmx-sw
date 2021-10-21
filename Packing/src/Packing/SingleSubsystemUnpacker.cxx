
#include "Packing/SingleSubsystemUnpacker.h"


namespace packing {

void SingleSubsystemUnpacker::beforeNewRun(ldmx::RunHeader& rh) {
  rh.setDetectorName(detector_name_);
}

void SingleSubsystemUnpacker::configure(framework::config::Parameters& ps) {
  reader_.open(ps.getParameter<std::string>("raw_file"));
  num_bytes_per_event_ = ps.getParameter<int>("num_bytes_per_event");
  output_name_ = ps.getParameter<std::string>("output_name");
  detector_name_=ps.getParameter<std::string>("detector_name");
}

void SingleSubsystemUnpacker::produce(framework::Event& event) {
  if (!reader_ or reader_.eof()) abortEvent();

  std::vector<uint8_t> buff;
  if(!reader_.read(buff, num_bytes_per_event_)) {
    EXCEPTION_RAISE("MalForm",
      "Raw file provided was unable to read "+std::to_string(num_bytes_per_event_)
      +" bytes in an event.");
  }
  event.add(output_name_, buff);
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, SingleSubsystemUnpacker)
