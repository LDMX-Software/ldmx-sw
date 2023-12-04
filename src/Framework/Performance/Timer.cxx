
#include "Framework/Performance/Timer.h"

#include "TVectorD.h"

ClassImp(framework::performance::Timer);

namespace framework::performance {

void Timer::reset() {
  start_ = {};
  end_ = {};
  start_time_ = -1;
  duration_ = -1;
}

void Timer::start() {
  start_ = clock::now();
  start_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(start_.time_since_epoch()).count();
}

void Timer::end() {
  end_ = clock::now();
  duration_ = std::chrono::duration<double>(end_ - start_).count();
}

double Timer::duration() const {
  return duration_;
}

void Timer::write(TDirectory* location, const std::string& name) const {
  location->WriteObject(this, name.c_str());
}

}
