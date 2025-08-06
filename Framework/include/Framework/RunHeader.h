#ifndef FRAMEWORK_EVENT_RUNHEADER_H_
#define FRAMEWORK_EVENT_RUNHEADER_H_

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TObject.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>
#include <string>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Version.h"

namespace ldmx {

/**
 * Run-specific configuration and data stored in its own output TTree alongside
 * the event TTree in the output file.
 *
 * Similar to the EventHeader, the evolution of this object has been pretty slow
 * since the `*Parameter*` members have been used to hold most of the additional
 * information (for example, a lot of the simulation configuration information).
 * The versions of the RunHeader as defined by ROOT's serialization
 * infrastructure are documented here.
 *
 * ## v1
 * An initial beta version now lost to the sands of time.
 *
 * ## v2 and v3
 * There are no material (serialized data) changes that are different between v2
 * and v3, but this version was increased because we moved the Framework repo
 * (and thus the RunHeader source) from being within ldmx-sw to its own
 * stand-alone repo. With an abundance of caution (and lack of understanding of
 * what ROOT's dictionary cares about), we increased the version number.
 *
 * ## v4
 * Add the numTries_ member variable in order to store exactly how many events
 * were started during the production of the run.
 *
 * ### Interop with v3
 * When reading a file written with v3 RunHeader using software with v4
 * RunHeader, the numTries_ member will keep its default value of 0. This is a
 * nice signal value since it conveys to the user the lack of information. If
 * the user somehow gets into the situation of reading a v4 RunHeader with v3
 * software, the numTries_ member is quietly ignored, maintaining the format of
 * the v3 RunHeader in the resulting output file.
 *
 * ## v5
 * Include a member to track the ldmx-sw version
 */
class RunHeader {
 public:
  /**
   * Constructor.
   *
   * @param runNumber The run number uniquely identifying this run
   */
  RunHeader(int runNumber);

  /**
   * Default constructor.
   *
   * @note This exists for filling the object from a ROOT branch.
   */
  RunHeader() = default;

  /** Destructor. */
  virtual ~RunHeader() {}

  /** @return The run number. */
  int getRunNumber() const { return run_number_; }

  /** @return The name of the detector used to create the events. */
  const std::string &getDetectorName() const { return detector_name_; }

  /** Set the name of the detector that was used in this run */
  void setDetectorName(const std::string &det) { detector_name_ = det; }

  /**
   * @return The git SHA-1 associated with the software tag used
   * to generate this file.
   */
  const std::string &getSoftwareTag() const { return software_tag_; }

  /**
   * @return The ldmx-sw version used to generate this file
   */
  const std::string &getLdmxswVersion() const { return ldmxsw_version_; }

  /** @return A short description of the run. */
  const std::string &getDescription() const { return description_; }

  /** Set the description of this run */
  void setDescription(const std::string &des) { description_ = des; }

  /**
   * Get the start time of the run in seconds since epoch.
   *
   * @return The start time of the run.
   *
   */
  int getRunStart() const { return run_start_; }

  /**
   * Set the run start time in seconds since epoch.
   *
   * @param[in] runStart the start time of the run.
   */
  void setRunStart(const int runStart) { run_start_ = runStart; }

  /**
   * Get the end time of the run in seconds since epoch.
   *
   * @return The end time of the run.
   */
  int getRunEnd() const { return run_end_; }

  /**
   * Set the end time of the run in seconds since epoch
   *
   * @param[in] runEnd the end time of the run.
   */
  void setRunEnd(const int runEnd) { run_end_ = runEnd; }

  /**
   * Get the total number of tries that were done during the production
   * of this run
   *
   * @return the number of tries
   */
  int getNumTries() const { return num_tries_; }

  /**
   * Set the total number of tries that were done during the production
   * of this run
   *
   * @note This function is called within framework::Process::run and
   * so it should not be used elsewhere. Changing the number of tries
   * during processing is undefined behavior.
   *
   * @param[in] numTries the number of tries in this run
   */
  void setNumTries(const int numTries) { num_tries_ = numTries; }

  /**
   * Get an int parameter value.
   *
   * @param name The name of the parameter.
   * @return The parameter value.
   */
  int getIntParameter(const std::string &name) const {
    return int_parameters_.at(name);
  }

  /// Get a const reference to all int parameters
  const std::map<std::string, int> &getIntParameters() const {
    return int_parameters_;
  }

  /**
   * Set an int parameter value.
   *
   * @param name The name of the parameter.
   * @param value The value of the parameter.
   */
  void setIntParameter(const std::string &name, int value) {
    int_parameters_[name] = value;
  }

  /**
   * Get a float parameter value.
   *
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  float getFloatParameter(const std::string &name) const {
    return float_parameters_.at(name);
  }

  /// Get a const reference to all float parameters
  const std::map<std::string, float> &getFloatParameters() const {
    return float_parameters_;
  }

  /**
   * Set a float parameter value.
   *
   * @param name The name of the parameter.
   * @param value The parameter value.
   */
  void setFloatParameter(const std::string &name, float value) {
    float_parameters_[name] = value;
  }

  /**
   * Get a string parameter value.
   *
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  std::string getStringParameter(const std::string &name) const {
    return string_parameters_.at(name);
  }

  /// Get a const reference to all string parameters
  const std::map<std::string, std::string> &getStringParameters() const {
    return string_parameters_;
  }

  /**
   * Set a string parameter value.
   *
   * @param name The name of the parameter.
   * @param value The parameter value.
   */
  void setStringParameter(const std::string &name, std::string value) {
    string_parameters_[name] = value;
  }

  /**
   * Stream this object into the input ostream
   *
   * Includes new-line characters to separate out the different parameter maps
   *
   * @param[in] s ostream to write to
   */
  void stream(std::ostream &s) const;

  /** Print a string desciption of this object. */
  void print() const;

  /**
   * Stream this object to an output stream
   *
   * Needs to be here and labeled as friend for
   * it to be compatible with Boost logging.
   *
   * @see ldmx::RunHeader::stream
   * @param[in] s ostream to write to
   * @param[in] h RunHeader to write out
   * @return modified ostream
   */
	  friend std::ostream &operator<<(std::ostream &s, const ldmx::RunHeader &h) {
    h.stream(s);
    return s;
  }

 private:
  /** Run number. */
  int run_number_{0};

  /** Detector name. */
  std::string detector_name_{""};

  /** Run description. */
  std::string description_{""};

  /// Run start in seconds since epoch
  int run_start_{0};

  /// Run end in seconds since epoch
  int run_end_{0};

  /**
   * Total number of events that were begun during the production
   * of this run
   *
   * This value is only set at the end of processing so we can faithfully
   * store the total number of events that were started. In the case where
   * maxTriesPerEvent is set to 1, this will be the same as the configured
   * maxEvents during Production Mode. If more than one try per event is
   * allowed, this will not necessarily be maxEvents since processors could
   * abort an event causing more than maxEvents to be started. This can be
   * summarized by the following inequality
   *
   * maxEvents <= numTries <= maxEvents*maxTriesPerEvent
   */
  int num_tries_{0};

  /**
   * git SHA-1 hash associated with the software tag used to generate
   * this file.
   */
  std::string software_tag_{GIT_SHA1};

  /**
   * ldmx-sw software version
   */
  std::string ldmxsw_version_{LDMXSW_VERSION};

  /** Map of int parameters. */
  std::map<std::string, int> int_parameters_;

  /** Map of float parameters. */
  std::map<std::string, float> float_parameters_;

  /** Map of string parameters. */
  std::map<std::string, std::string> string_parameters_;

  ClassDef(RunHeader, 5);

};  // RunHeader

}  // namespace ldmx

#endif  // _FRAMEWORK_EVENT_RUN_HEADER_H_
