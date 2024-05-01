#ifndef FRAMEWORK_PERFORMANCE_CALLBACK
#define FRAMEWORK_PERFORMANCE_CALLBACK

#include <string>

namespace framework::performance {

/**
 * Identification of specific processor callbacks
 *
 * The names generally correspond to the name of the processor function
 * except the process value which corresponds to produce for Producers
 * and analyze for Analyzers
 */
enum class Callback {
  onProcessStart = 0,
  onProcessEnd = 1,
  onFileOpen = 2,
  onFileClose = 3,
  beforeNewRun = 4,
  onNewRun = 5,
  process = 6
};

/**
 * Convert the Callback enum into an index for lookup
 */
constexpr std::size_t to_index(Callback c) {
  return static_cast<std::size_t>(c);
}

/**
 * Convert the Callback enum into a human-readable name
 */
std::string to_name(Callback c);

}  // namespace framework::performance

#endif
