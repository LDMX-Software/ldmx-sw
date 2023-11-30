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
  /// default constructor (i.e. initialize values to defaults)
  Measurement() = default;
  /**
   * constructor giving caller ability to do the sampling immediately
   *
   * @see sample for how sampling is done
   * @param[in] do_sample if true, call sample immediately
   */
  Measurement(bool do_sample);
  /// virtual destructor for ROOT serialization
  virtual ~Measurement() = default;
  /**
   * Take and store the measurment
   */
  void sample();
  /**
   * Set the validity of this measurement to false
   *
   * This is helpful in the case where we want to have one measurement object
   * and serialize multiple measurements from it (i.e. when taking the same measurment
   * for each successive events)
   */
  void invalidate();
 private:
  /// validity of this measurement (true if `sample` was called, false otherwise)
  bool valid_{false};
  /// the time stamp of this measurement (as measured by system clock)
  long unsigned int time_;
  ClassDef(Measurement, 1);
};
}

#endif
