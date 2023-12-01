
#include "Framework/Performance/Timer.h"

#include "TVectorD.h"

namespace framework::performance {

Timer::Timer(bool do_start) {
  if (do_start) start();
}

void Timer::start() {
  start_ = clock::now();
}

void Timer::end() {
  end_ = clock::now();
}

double Timer::duration() const {
  return std::chrono::duration<double>(end_ - start_).count();
}

void Timer::write(TDirectory* location, const std::string& name) const {
  /**
   * The official ROOT method for writing a single data point into a file
   * is to wrap it as a single-element array. ¯\_(ツ)_/¯
   *
   * brun approved
   * https://root-forum.cern.ch/t/writing-simple-variable-in-root-files/11094
   */
  TVectorD v(1);
  v[0] = duration();
  location->WriteObject(&v, name.c_str());
}

}
