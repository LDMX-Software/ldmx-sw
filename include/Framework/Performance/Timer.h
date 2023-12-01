#ifndef FRAMEWORK_PERFORMANCE_TIMER
#define FRAMEWORK_PERFORMANCE_TIMER

#include <chrono>
#include <string>

#include "TDirectory.h"

namespace framework::performance {

class Timer {
  using clock = std::chrono::high_resolution_clock;
  std::chrono::time_point<clock> start_;
  std::chrono::time_point<clock> end_;
 public:
  Timer() = default;
  Timer(bool do_start);
  void start();
  void end();
  double duration() const;
  void write(TDirectory* location, const std::string& name) const;
};

}

#endif
