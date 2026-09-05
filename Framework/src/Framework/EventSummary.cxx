
#include "Framework/EventSummary.h"

#include <iostream>

ClassImp(ldmx::EventSummary);

namespace ldmx {
const std::string EventSummary::BRANCH = "EventSummary";

void EventSummary::clear(Option_t*) {
  event_number_ = 0;
  timestamp_ns_ = 0;
  nsystems_ = 0;
  system_ids_.clear();
  payload_size_ = 0;
  error_flags_ = 0;
}

std::ostream& operator<<(std::ostream& o, const EventSummary& c) {
  return o << "EventSummary { " << "eventNumber: " << c.getEventNumber()
           << ", timestamp: " << c.getTimestampNs() << " ns"
           << ", nSystems: " << c.getNSystems()
           << ", payloadSize: " << c.getPayloadSize() << " bytes"
           << ", errorFlags: 0x" << std::hex << c.getErrorFlags() << std::dec
           << " }";
}

}  // namespace ldmx
