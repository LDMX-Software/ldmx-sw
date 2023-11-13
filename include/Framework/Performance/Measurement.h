#ifndef FRAMEWORK_PERFORMANCE_MEASUREMENT
#define FRAMEWORK_PERFORMANCE_MEASUREMENT

#include <TObject.h>

/**
 * Namespace keeping performance-related classes all together
 */
namespace framework::performance {

/**
 * A single measurement taken at one time.
 *
 * We use ROOT's macros to make sure this class is serializable,
 * but it is /not/ included in the normal Event/ dictionary so that
 * it can evolve separately from it.
 */
class Measurement {
 public:
  Measurement() = default;
  virtual ~Measurement() = default;
  void sample();
 private:
  int time_;
  int mem_;
  int cpu_;
  ClassDef(Measurement, 1);
};
}

#endif
