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
   * constructor giving caller ability to start the timer immediately
   *
   * @see start for how the timer is started
   * @param[in] start if true, call begin immediately
   */
  Measurement(bool start);
  /// virtual destructor for ROOT serialization
  virtual ~Measurement() = default;
  /**
   * Start the timer
   */
  void start();
  /**
   * End the timer
   */
  void end();
  /**
   * Set the validity of this measurement to false
   *
   * This is helpful in the case where we want to have one measurement object
   * and serialize multiple measurements from it (i.e. when taking the same measurment
   * for each successive events)
   */
  void invalidate();
 private:
  /// validity of this measurement (true if `end` was called, false otherwise)
  bool valid_{false};
  /// the time stamp of when this timer started (as measured by system clock)
  long unsigned int start_;
  /// how long the timer ran in ns
  long unsigned int duration_;
  ClassDef(Measurement, 1);
};
}

#endif
