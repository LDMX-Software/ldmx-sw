
#include "Framework/EventHeader.h"

#include "Framework/Exception/Exception.h"

ClassImp(ldmx::EventHeader);

namespace ldmx {
const std::string EventHeader::BRANCH = "EventHeader";

void EventHeader::clear(Option_t*) {
  event_number_ = -1;
  run_ = -1;
  timestamp_ = TTimeStamp(0, 0);
  weight_ = 1.0;
  is_real_data_ = false;
  int_parameters_.clear();
  float_parameters_.clear();
  string_parameters_.clear();
}

void EventHeader::print(Option_t*) const {
  std::cout << "EventHeader {" << " eventNumber: " << event_number_
            << ", run: " << run_ << ", timestamp: " << timestamp_
            << ", weight: " << weight_;
  if (is_real_data_) {
    std::cout << ", DATA";
  }
  else {
    std::cout << ", MC";
  }
  std::cout << " }" << std::endl;
}

int EventHeader::getIntParameter(const std::string& name) const {
  if (int_parameters_.find(name) == int_parameters_.end()) {
    EXCEPTION_RAISE("NoParam", "Parameter '" + name +
                                   "' does not exist in the int parameters.");
  }
  return int_parameters_.at(name);
}

float EventHeader::getFloatParameter(const std::string& name) const {
  if (float_parameters_.find(name) == float_parameters_.end()) {
    EXCEPTION_RAISE("NoParam", "Parameter '" + name +
                                   "' does not exist in the float parameters.");
  }
  return float_parameters_.at(name);
}

std::string EventHeader::getStringParameter(const std::string& name) const {
  if (string_parameters_.find(name) == string_parameters_.end()) {
    EXCEPTION_RAISE(
        "NoParam",
        "Parameter '" + name + "' does not exist in the string parameters.");
  }
  return string_parameters_.at(name);
}

}  // namespace ldmx
