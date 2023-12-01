#ifndef FRAMEWORK_PERFORMANCE_CALLBACK
#define FRAMEWORK_PERFORMANCE_CALLBACK

#include <vector>

namespace framework::performance {

enum class Callback {
  onProcessStart = 0,
  onProcessEnd   = 1,
  onFileOpen     = 2,
  onFileClose    = 3,
  beforeNewRun   = 4,
  onNewRun       = 5,
  process        = 6
};

std::size_t to_index(Callback c);

const char* to_name(Callback c);

}

#endif

