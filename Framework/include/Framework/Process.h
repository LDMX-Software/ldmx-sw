/**
 * @file Process.h
 * @brief Class which represents the process under execution.
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef LDMXSW_FRAMEWORK_PROCESS_H_
#define LDMXSW_FRAMEWORK_PROCESS_H_

// LDMX
#include "Framework/Conditions.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Performance/Tracker.h"
#include "Framework/RunHeader.h"
#include "Framework/StorageControl.h"

// STL
#include <csignal>
#include <map>
#include <memory>
#include <vector>

class TFile;
class TDirectory;

namespace framework {

class EventProcessor;
class EventFile;
class Event;

/**
 * @class Process
 * @brief Class which represents the process under execution.
 */
class Process {
 public:
  /**
   * Class constructor.
   * @param configuration Parameters to configure process with
   */
  Process(const framework::config::Parameters &configuration);

  /**
   * Class Destructor
   *
   * Cleans up sequence of EventProcessors.
   * These processors were created by ConfigurePython and should be deleted.
   */
  ~Process();

  /**
   * Get the processing pass label.
   * @return The processing pass label.
   */
  const std::string &getPassName() const { return pass_name_; }

  /**
   * Get the current run number or the run number to be used when initiating new
   * events from the job
   * @return int Run number
   */
  int getRunNumber() const;

  /**
   * Get the pointer to the current event header, if defined
   */
  const ldmx::EventHeader *getEventHeader() const { return event_header_; }

  /**
   * Get the pointer to the current run header, if defined
   */
  const ldmx::RunHeader *getRunHeader() const { return run_header_; }

  /**
   * Get a reference to the conditions system
   */
  Conditions &getConditions() { return conditions_; }

  /**
   * Get the frequency with which the event information is printed.
   * @return integer log frequency (negative if turned off)
   */
  int getLogFrequency() const { return log_frequency_; }

  /**
   * Run the process.
   */
  void run();

  /**
   * Request that the processing finish with this event
   */
  void requestFinish() { event_limit_ = 0; }

  /**
   * Construct a TDirectory* for the given module
   */
  TDirectory *makeHistoDirectory(const std::string &dirName);

  /**
   * Open a ROOT TFile to write histograms and TTrees.
   */
  TDirectory *openHistoFile();

  /**
   * Access the storage control unit for this process
   */
  StorageControl &getStorageController() { return storage_controller_; }

  /**
   * Set the pointer to the current event header, used only for tests
   */
  void setEventHeader(ldmx::EventHeader *h) { event_header_ = h; }

 private:
  /**
   * Process the input event through the sequence
   * of processors
   *
   * The input counters (for events and tries) are
   * only used to print the status.
   *
   * @param[in] n counter for number of events processed
   * @param[in] n_tries counter for number of tries on current event
   * @param[in,out] event reference to event we are going to process
   * @returns true if event was full processed (false if aborted)
   */
  bool process(int n, int n_tries, Event &event) const;

  /**
   * Run through the processors and let them know
   * that we are starting a new run.
   *
   * @param[in] header RunHeader for the new run
   */
  void newRun(ldmx::RunHeader &header);

  /**
   * File is being opened
   */
  void onFileOpen(EventFile &file) const;

  /**
   * File is begin closed
   */
  void onFileClose(EventFile &file) const;

 private:
  /// The parameters used to configure this class.
  framework::config::Parameters config_;

  /** Processing pass name. */
  std::string pass_name_;

  /** Limit on events to process. */
  int event_limit_;

  /** When reading a file in, what's the first event to read */
  int min_events_;

  /** Number of events we'd like to produce
   independetly of the number of tries it would take.
   Be warned about infinite loops!*/
  int total_events_;

  /** The frequency with which event info is printed. */
  int log_frequency_;

  /** Maximum number of attempts to make before giving up on an event */
  int max_tries_;

  /**
   * allow the Process to skip input files that are corrupted
   */
  bool skip_corrupted_input_files_;

  /** Storage controller */
  StorageControl storage_controller_;

  /** Ordered list of EventProcessors to execute. */
  std::vector<EventProcessor *> sequence_;

  /** Set of ConditionsProviders */
  Conditions conditions_;

  /** List of input files to process.  May be empty if this Process will
   * generate new events. */
  std::vector<std::string> input_files_;

  /** List of output file names.  If empty, no output file will be created. */
  std::vector<std::string> output_files_;

  /** Compression setting to pass to output files
   *
   * Look at the documentation for the TFile constructor if you
   * want to learn more details. Essentially,
   * setting = 100*algo + level
   * with algo = 0 being the global default.
   */
  int compression_setting_;

  /** Set of drop/keep rules. */
  std::vector<std::string> drop_keep_rules_;

  /** Run number to use if generating events. */
  int run_for_generation_{1};

  /** Filename for histograms and other user products */
  std::string histo_filename_;

  /** Pointer to the current EventHeader, used for Conditions information */
  const ldmx::EventHeader *event_header_{0};

  /** Pointer to the current RunHeader, used for Conditions information */
  ldmx::RunHeader *run_header_{0};

  /** TFile for histograms and other user products */
  TFile *histo_t_file_{0};

  /** class with calls backs to track performance measurements of software */
  performance::Tracker *performance_{0};

  /// Turn on logging for our process
  enableLogging("Process");
};

/**
 * A handle to the current process
 * Used to pass a process from ConfigurePython
 * to fire.cxx
 */
typedef std::unique_ptr<Process> ProcessHandle;
}  // namespace framework

#endif
