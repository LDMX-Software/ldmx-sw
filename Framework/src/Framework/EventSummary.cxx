
#include "Framework/EventSummary.h"

#include <iostream>

ClassImp(ldmx::EventSummary);

namespace ldmx {
const std::string EventSummary::BRANCH = "EventSummary";

void EventSummary::clear(Option_t*) {
  event_id_ = 0;
  timestamp_ns_ = 0;
  nsystems_ = 0;
  system_ids_.clear();
  payload_size_ = 0;
  error_flags_ = 0;
}

void EventSummary::print(Option_t*) const {
  std::cout << "EventSummary {" << " eventId: " << event_id_
            << ", timestamp: " << timestamp_ns_ << " ns"
            << ", nSystems: " << nsystems_
            << ", payloadSize: " << payload_size_ << " bytes"
            << ", errorFlags: 0x" << std::hex << error_flags_ << std::dec;
  std::cout << " }" << std::endl;
}

}  // namespace ldmx
