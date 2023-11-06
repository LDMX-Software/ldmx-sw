#pragma once

namespace framework {
/**
 * Log performance data into a specific TTree
 */
class PerformanceLogger {
 public:
  /**
   * Create the performance logger with a ROOT directory in which
   * the data should be stored.
   */
  PerformanceLogger(TDirectory* storage_directory);

};  // class PerformanceLogger

}  // namespace framework
