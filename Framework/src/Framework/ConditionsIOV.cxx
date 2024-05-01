#include "Framework/ConditionsIOV.h"

#include <iostream>
#include <sstream>

#include "Framework/EventHeader.h"

std::ostream& operator<<(std::ostream& s, const framework::ConditionsIOV& iov) {
  iov.stream(s);
  return s;
}

namespace framework {

bool ConditionsIOV::validForEvent(const ldmx::EventHeader& eh) const {
  return (eh.getRun() >= firstRun_ || firstRun_ == -1) &&
         (eh.getRun() <= lastRun_ || lastRun_ == -1) &&
         ((eh.isRealData()) ? (validForData_) : (validForMC_));
}

bool ConditionsIOV::overlaps(const ConditionsIOV& iov) const {
  if (iov.validForData_ != validForData_ && iov.validForMC_ != validForMC_)
    return false;
  if (iov.firstRun_ < lastRun_) return false;  // starts after this IOV
  if (iov.lastRun_ < firstRun_) return false;  // ends before this IOV
  return true;
}

void ConditionsIOV::Print() const {
  stream(std::cout);
  std::cout << std::endl;
}

std::string ConditionsIOV::ToString() const {
  std::stringstream s;
  stream(s);
  return s.str();
}

void ConditionsIOV::stream(std::ostream& s) const {
  s << "IOV(" << firstRun_ << "->";
  if (lastRun_ == -1)
    s << "[all runs]";
  else
    s << lastRun_;
  if (validForData_) s << ",Data";
  if (validForMC_) s << ",MC";
  s << ")";
}
}  // namespace framework
