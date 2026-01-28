
#include "Packing/SingleSubsystemUnpackerFixedLength.h"

namespace packing {

void SingleSubsystemUnpackerFixedLength::beforeNewRun(ldmx::RunHeader& rh) {
  rh.setDetectorName(detector_name_);
}

void SingleSubsystemUnpackerFixedLength::configure(
    framework::config::Parameters& ps) {
  reader_.open(ps.get<std::string>("raw_file"));
  num_bytes_per_event_ = ps.get<int>("num_bytes_per_event");
  output_name_ = ps.get<std::string>("output_name");
  detector_name_ = ps.get<std::string>("detector_name");
}

void SingleSubsystemUnpackerFixedLength::produce(framework::Event& event) {
  if (!reader_ or reader_.eof()) abortEvent();

  std::vector<uint8_t> buff;
  if (!reader_.read(buff, num_bytes_per_event_)) {
    EXCEPTION_RAISE("MalForm", "Raw file provided was unable to read " +
                                   std::to_string(num_bytes_per_event_) +
                                   " bytes in an event.");
  }
  event.add(output_name_, buff);
}

}  // namespace packing

DECLARE_PRODUCER(packing::SingleSubsystemUnpackerFixedLength)
