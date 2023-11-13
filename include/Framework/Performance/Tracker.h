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
  PerformanceTracker(TDirectory *storage_directory);
  // destructor needs to make sure that the trees/objects are written
  // so that Process can just delete it when closing
  ~PerformanceTracker();
  /* begin list of callbacks for various points in Process::run */
  void absolute_start(); // literally first line of Process::run
  void absolute_end(); // literally last line of Process::run (only called when run compeltes without errors)
  void begin_onProcessStart(); // before onProcessStart section
  void end_onProcessStart(); // after onProcessStart section
  void begin_onProcessStart(const std::string& processor); // before processor specific onProcessStart
  void end_onProcessStart(const std::string& processor); // after processor specific onProcessStart
  // similar callbacks for the different EventProcessor callbacks
 private:
  // has some handle to the destination for the data
  TDirectory *storage_directory_;
  // has a TTree for event-by-event perf info
  TTree *event_data_;

  Measurement absolute_start_, absolute_end_, begin_onProcessStart_, end_onProcessStart_;
  std::map<std::string, Measurement> 
    begin_onProcessStart_processors,
    end_onProcessStart_processors;
};
}

#endif
