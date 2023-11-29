#ifndef FRAMEWORK_PERFORMANCE_TRACKER
#define FRAMEWORK_PERFORMANCE_TRACKER

#include <map>

#include <TTree.h>
#include <TDirectory.h>

#include "Framework/Performance/Measurement.h"

namespace framework::performance {

/**
 * Class to interface between framework::Process and various Measurments
 * that can eventually be written into the output histogram file.
 */
class Tracker {
 public:
  static const std::string ALL;
  // create it with the destination
  // e.g. with Process::makeHistoDirectory("performance")
  Tracker(TDirectory *storage_directory);
  // destructor needs to make sure that the trees/objects are written
  // so that Process can just delete it when closing
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
  // has some handle to the destination for the data
  TDirectory *storage_directory_;
  // has a TTree for event-by-event perf info
  TTree *event_data_;

  Measurement absolute_start_, absolute_end_;
  std::map<std::string, Measurement> 
    begin_onProcessStart_,
    end_onProcessStart_,
    begin_onProcessEnd_,
    end_onProcessEnd_,
    begin_onFileOpen_,
    end_onFileOpen_,
    begin_onFileClose_,
    end_onFileClose_,
    begin_beforeNewRun_,
    end_beforeNewRun_,
    begin_onNewRun_,
    end_onNewRun_;
  std::map<std::string, Measurement*>
    begin_process_,
    end_process_;
};
}

#endif
