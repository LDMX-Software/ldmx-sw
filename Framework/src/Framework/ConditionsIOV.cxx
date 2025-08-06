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
  return (eh.getRun() >= first_run_ || first_run_ == -1) &&
         (eh.getRun() <= last_run_ || last_run_ == -1) &&
         ((eh.isRealData()) ? (valid_for_data_) : (valid_for_mc_));
}

bool ConditionsIOV::overlaps(const ConditionsIOV& iov) const {
  if (iov.valid_for_data_ != valid_for_data_ &&
      iov.valid_for_mc_ != valid_for_mc_)
    return false;
  if (iov.first_run_ < last_run_) return false;  // starts after this IOV
  if (iov.last_run_ < first_run_) return false;  // ends before this IOV
  return true;
}

void ConditionsIOV::print() const {
  stream(std::cout);
  std::cout << std::endl;
}

std::string ConditionsIOV::toString() const {
  std::stringstream s;
  stream(s);
  return s.str();
}

void ConditionsIOV::stream(std::ostream& s) const {
  s << "IOV(" << first_run_ << "->";
  if (last_run_ == -1)
    s << "[all runs]";
  else
    s << last_run_;
  if (valid_for_data_) s << ",Data";
  if (valid_for_mc_) s << ",MC";
  s << ")";
}
}  // namespace framework
