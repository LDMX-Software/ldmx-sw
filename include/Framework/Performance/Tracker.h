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
  void begin_onProcessStart(const std::string& processor); // before processor specific onProcessStart
  void end_onProcessStart(const std::string& processor); // after processor specific onProcessStart
  
  // similar callbacks for the different EventProcessor callbacks
 private:
  // has some handle to the destination for the data
  TDirectory *storage_directory_;
  // has a TTree for event-by-event perf info
  TTree *event_data_;

  Measurement absolute_start_, absolute_end_;
  std::map<std::string, Measurement> 
    begin_onProcessStart_,
    end_onProcessStart_;
};
}

#endif
