
#include "Framework/EventHeader.h"
#include "Framework/Exception/Exception.h"

ClassImp(ldmx::EventHeader);

namespace ldmx {
const std::string EventHeader::BRANCH = "EventHeader";

void EventHeader::Clear(Option_t*) {
  eventNumber_ = -1;
  run_ = -1;
  timestamp_ = TTimeStamp(0, 0);
  weight_ = 1.0;
  tries_ = 0;
  isRealData_ = false;
  intParameters_.clear();
  floatParameters_.clear();
  stringParameters_.clear();
}

void EventHeader::Print(Option_t*) const {
  std::cout << "EventHeader {"
            << " eventNumber: " << eventNumber_ << ", run: " << run_
            << ", timestamp: " << timestamp_ << ", weight: " << weight_
            << ", tries: " << tries_;
  if (isRealData_)
    std::cout << ", DATA";
  else
    std::cout << ", MC";
  std::cout << " }" << std::endl;
}

int EventHeader::getIntParameter(const std::string& name) const {
  if (intParameters_.find(name) == intParameters_.end()) {
    EXCEPTION_RAISE("NoParam",
        "Parameter '"+name+"' does not exist in the int parameters.");
  }
  return intParameters_.at(name);
}

float EventHeader::getFloatParameter(const std::string& name) const {
  if (floatParameters_.find(name) == floatParameters_.end()) {
    EXCEPTION_RAISE("NoParam",
        "Parameter '"+name+"' does not exist in the float parameters.");
  }
  return floatParameters_.at(name);
}

std::string EventHeader::getStringParameter(const std::string& name) const {
  if (stringParameters_.find(name) == stringParameters_.end()) {
    EXCEPTION_RAISE("NoParam",
        "Parameter '"+name+"' does not exist in the string parameters.");
  }
  return stringParameters_.at(name);
}

}
