#include "Framework/Performance/Measurement.h"

#include <chrono>

ClassImp(framework::performance::Measurement);

namespace framework::performance {

static long unsigned int get_current_time() {
  using std::chrono::system_clock;
  const auto current_time{system_clock::now()};
  return system_clock::to_time_t(current_time);
}

Measurement::Measurement(bool do_start) {
  if (do_start) start();
}

void Measurement::start() {
  start_ = get_current_time();
}

void Measurement::end() {
  valid_ = true;
  duration_ = get_current_time() - start_;
}

void Measurement::invalidate() {
  valid_ = false;
}

}
