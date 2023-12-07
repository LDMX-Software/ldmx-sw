#ifndef FRAMEWORK_PERFORMANCE_TRACKER
#define FRAMEWORK_PERFORMANCE_TRACKER

#include <map>

#include <TTree.h>
#include <TDirectory.h>

#include "Framework/Performance/Timer.h"
#include "Framework/Performance/Callback.h"

namespace framework::performance {

/**
 * Class to interface between framework::Process and various measurements
 * that can eventually be written into the output histogram file.
 *
 * @see Timer for the data format of timing measurements
 */
class Tracker {
 public:
  /**
   * Create the tracker with a specific destination for writing information
   *
   * @param[in] storage_directory directory in-which to write data when closing
   * @param[in] names sequence of processor names we will be tracking
   */
  Tracker(TDirectory *storage_directory, const std::vector<std::string>& names);
  /**
   * Close up tracking and write all of the data collected to the storage directory
   */
  ~Tracker();
  /// literally first line of Process::run
  void absolute_start();
  /// literally last line of Process::run (if run compeletes without error)
  void absolute_stop();
  /// start the timer for a specific callback and specific processor
  void start(Callback cb, std::size_t i_proc);
  /// stop the timer for a specific callback and specific processor
  void stop(Callback cb, std::size_t i_proc);
  /// inform us that we finished an event (and whether it was completed or not)
  void end_event(bool completed);

 private:
  /**
   * Special name representing "all" processors in the sequence.
   * For measurements related to beginning, this is before all
   * processors and for measurements related to ending, this is
   * after all processors.
   */
  static const std::string ALL;
  /// handle to the destination for the data
  TDirectory *storage_directory_;
  /// event-by-event perf info
  TTree *event_data_;
  /// buffer for bool flag on if event completed
  bool event_completed_;

  /// timer from the first line of Process::run to the last line
  Timer absolute_;
  /**
   * a timer for each processor in the sequence and each callback
   *
   * The timers for the process Callback (Producer::produce or Analyzer::analyze)
   * need to stay in the same memory location during the lifetime of this
   * object. This is because the address of those timers is given
   * to event_data_ to watch (and eventually serialize) while processing.
   * Effectively, this means the size and shape of this member should be
   * set in the constructor and then it should not be changed.
   */
  std::vector<std::vector<Timer>> processor_timers_;
  /// names of the processors in the sequence for serialization
  std::vector<std::string> names_;
};
}

#endif
