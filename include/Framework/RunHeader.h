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
  int getRunNumber() const { return runNumber_; }

  /** @return The name of the detector used to create the events. */
  const std::string &getDetectorName() const { return detectorName_; }

  /** Set the name of the detector that was used in this run */
  void setDetectorName(const std::string &det) { detectorName_ = det; }

  /**
   * @return The git SHA-1 associated with the software tag used
   * to generate this file.
   */
  const std::string &getSoftwareTag() const { return softwareTag_; }

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
  int getRunStart() const { return runStart_; }

  /**
   * Set the run start time in seconds since epoch.
   *
   * @param[in] runStart the start time of the run.
   */
  void setRunStart(const int runStart) { runStart_ = runStart; }

  /**
   * Get the end time of the run in seconds since epoch.
   *
   * @return The end time of the run.
   */
  int getRunEnd() const { return runEnd_; }

  /**
   * Set the end time of the run in seconds since epoch
   *
   * @param[in] runEnd the end time of the run.
   */
  void setRunEnd(const int runEnd) { runEnd_ = runEnd; }

  /**
   * Get the total number of tries that were done during the production
   * of this run
   *
   * @return the number of tries
   */
  int getNumTries() const { return numTries_; }

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
  void setNumTries(const int numTries) { numTries_ = numTries; }

  /**
   * Get an int parameter value.
   *
   * @param name The name of the parameter.
   * @return The parameter value.
   */
  int getIntParameter(const std::string &name) const {
    return intParameters_.at(name);
  }

  /// Get a const reference to all int parameters
  const std::map<std::string, int> &getIntParameters() const {
    return intParameters_;
  }

  /**
   * Set an int parameter value.
   *
   * @param name The name of the parameter.
   * @param value The value of the parameter.
   */
  void setIntParameter(const std::string &name, int value) {
    intParameters_[name] = value;
  }

  /**
   * Get a float parameter value.
   *
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  float getFloatParameter(const std::string &name) const {
    return floatParameters_.at(name);
  }

  /// Get a const reference to all float parameters
  const std::map<std::string, float> &getFloatParameters() const {
    return floatParameters_;
  }

  /**
   * Set a float parameter value.
   *
   * @param name The name of the parameter.
   * @param value The parameter value.
   */
  void setFloatParameter(const std::string &name, float value) {
    floatParameters_[name] = value;
  }

  /**
   * Get a string parameter value.
   *
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  std::string getStringParameter(const std::string &name) const {
    return stringParameters_.at(name);
  }

  /// Get a const reference to all string parameters
  const std::map<std::string, std::string> &getStringParameters() const {
    return stringParameters_;
  }

  /**
   * Set a string parameter value.
   *
   * @param name The name of the parameter.
   * @param value The parameter value.
   */
  void setStringParameter(const std::string &name, std::string value) {
    stringParameters_[name] = value;
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
  void Print() const;

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
  int runNumber_{0};

  /** Detector name. */
  std::string detectorName_{""};

  /** Run description. */
  std::string description_{""};

  /// Run start in seconds since epoch
  int runStart_{0};

  /// Run end in seconds since epoch
  int runEnd_{0};

  /**
   * Total number of events that were begun during the production
   * of this run
   *
   * This value is only set at the end of processing so we can faithfully
   * store the total number of events that were started. In the case where
   * maxTriesPerEvent is set to 1, this will be the same as the configured
   * maxEvents during Production Mode. If more than one try per event is allowed,
   * this will not necessarily be maxEvents since processors could abort an event
   * causing more than maxEvents to be started. This can be summarized by the
   * following inequality
   *
   * maxEvents <= numTries <= maxEvents*maxTriesPerEvent
   */
  int numTries_{0};

  /**
   * git SHA-1 hash associated with the software tag used to generate
   * this file.
   */
  std::string softwareTag_{GIT_SHA1};

  /** Map of int parameters. */
  std::map<std::string, int> intParameters_;

  /** Map of float parameters. */
  std::map<std::string, float> floatParameters_;

  /** Map of string parameters. */
  std::map<std::string, std::string> stringParameters_;

  ClassDef(RunHeader, 4);

};  // RunHeader

}  // namespace ldmx

#endif  // _FRAMEWORK_EVENT_RUN_HEADER_H_
