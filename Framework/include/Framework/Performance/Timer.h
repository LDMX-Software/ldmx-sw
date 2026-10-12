#ifndef FRAMEWORK_PERFORMANCE_TIMER
#define FRAMEWORK_PERFORMANCE_TIMER

#include <Rtypes.h>  // For ClassDef

#include <chrono>
#include <string>

#include "TDirectory.h"

namespace framework::performance {

/**
 * Time how long a specific operation takes and serialize
 * the result with ROOT
 *
 * Under-the-hood, we use std::chrono::time_point and std::chrono::duration
 * along with std::chrono::high_resolution_clock so we can clearly and
 * faithfully time how long things take and transparently convert the resulting
 * duration into seconds (including sub-second increments). Since ROOT doesn't
 * have a dictionary for serializing these std::chrono classes (and I'm not
 * interested in making one), I simply mark those helper-members as "transient"
 * (have `//!` on the line they are declared in) so they
 * are ignored by the dictionary generation. The fundamental members start_time_
 * and duration_ are what end up being written to disk.
 *
 * Below is an example of reading the absolute timer from a test run of this
 * performance logging infrastructure. As you can see, the two atomic types
 * are available for reading but the std::chrono::time_point objects are not.
 *
 * ```python
 * >>> import uproot
 * >>> with uproot.open('test_performance.root') as f:
 * ...     f['performance/absolute'].members
 * ...
 * {'start_time_': 1701704424941358702, 'duration_': 0.078910213}
 * ```
 */
class Timer {
  using clock = std::chrono::high_resolution_clock;
  /**
   * The time_point when the timer is started.
   *
   * The trailing `//!` comment marks this member as "transient"
   * for ROOT I/O and must stay on the same line as the member.
   */
  std::chrono::time_point<clock> begin_;  //! transient

  /**
   * The time_point when the timer is stopped.
   *
   * The trailing `//!` comment marks this member as "transient"
   * for ROOT I/O and must stay on the same line as the member.
   */
  std::chrono::time_point<clock> end_;  //! transient

  /**
   * Time stamp for when timer was started in nanoseconds since UNIX epoch
   *
   * Set to -1 if timer was not started
   */
  long int start_time_{-1};

  /**
   * Length of time recorded by the timer in seconds
   *
   * Sub-second intervals are recorded as well (so if the timer
   * ran for 1 millisecond you would see 0.001 stored in this
   * member variable).
   *
   * Set to -1 if timer was not ended
   */
  double duration_{-1};

 public:
  /// create a timer but don't start it yet
  Timer() = default;
  /// create defualt destructor
  virtual ~Timer() = default;
  /// reset a timer to un-started state without re-allocating
  void reset();
  /// start the timer
  void start();
  /// stop the timer
  void stop();
  /// retrieve the value of the duration in seconds
  double duration() const;
  /**
   * Write ourselves under the input name to the input location
   *
   * This is just here to avoid repeating the boiler-plate
   * ```cpp
   * location->WriteObject(&timer_obj, name.c_str());
   * ```
   * Since I don't like seeing `&` or `c_str()` in my code.
   */
  void write(TDirectory* location, const std::string& name) const;
  ClassDef(Timer, 2);
};

}  // namespace framework::performance

#endif
