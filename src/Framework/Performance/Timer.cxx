
#include "Framework/Performance/Timer.h"

ClassImp(framework::performance::Timer);

namespace framework::performance {

void Timer::reset() {
  begin_ = {};
  end_ = {};
  start_time_ = -1;
  duration_ = -1;
}

void Timer::start() {
  begin_ = clock::now();
  start_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    begin_.time_since_epoch())
                    .count();
}

void Timer::stop() {
  end_ = clock::now();
  duration_ = std::chrono::duration<double>(end_ - begin_).count();
}

double Timer::duration() const { return duration_; }

void Timer::write(TDirectory* location, const std::string& name) const {
  location->WriteObject(this, name.c_str());
}

}  // namespace framework::performance
