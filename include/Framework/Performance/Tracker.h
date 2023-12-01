#ifndef FRAMEWORK_PERFORMANCE_TRACKER
#define FRAMEWORK_PERFORMANCE_TRACKER

#include <map>

#include <TTree.h>
#include <TDirectory.h>

#include "Framework/Performance/Measurement.h"

namespace framework::performance {

/**
 * Class to interface between framework::Process and various Measurements
 * that can eventually be written into the output histogram file.
 *
 * @see Measurement for what measurements are taken and how they are taken.
 */
class Tracker {
 public:
  /**
   * Special name representing "all" processors in the sequence.
   * For measurements related to beginning, this is before all
   * processors and for measurements related to ending, this is
   * after all processors.
   */
  static const std::string ALL;
  /**
   * Create the tracker with a specific destination for writing information
   *
   * @param[in] storage_directory directory in-which to write data when closing
   */
  Tracker(TDirectory *storage_directory);
  /**
   * Close up tracking and write all of the data collected to the storage directory
   */
  ~Tracker();
  /* begin list of callbacks for various points in Process::run */
  /// literally first line of Process::run
  void absolute_start();
  /// literally last line of Process::run (if run compeletes without error)
  void absolute_end();
  void begin_onProcessStart(const std::string& processor);
  void end_onProcessStart(const std::string& processor);
  void begin_onFileOpen(const std::string& processor);
  void end_onFileOpen(const std::string& processor);
  void begin_onFileClose(const std::string& processor);
  void end_onFileClose(const std::string& processor);
  void begin_onProcessEnd(const std::string& processor);
  void end_onProcessEnd(const std::string& processor);
  void begin_beforeNewRun(const std::string& processor);
  void end_beforeNewRun(const std::string& processor);
  void begin_onNewRun(const std::string& processor);
  void end_onNewRun(const std::string& processor);
  void begin_process(const std::string& processor);
  void end_process(const std::string& processor);
  void end_event();

 private:
  /// handle to the destination for the data
  TDirectory *storage_directory_;
  /// event-by-event perf info
  TTree *event_data_;

  Measurement absolute_;
  std::map<std::string, Measurement> 
    onProcessStart_,
    onProcessEnd_,
    onFileOpen_,
    onFileClose_,
    beforeNewRun_,
    onNewRun_;
  std::map<std::string, Measurement*>
    process_;
};
}

#endif
