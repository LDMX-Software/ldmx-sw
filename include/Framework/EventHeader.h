/**
 * @file EventHeader.h
 * @brief Class that provides header information about an event such as event
 * number and timestamp
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_EVENTHEADER_H_
#define EVENT_EVENTHEADER_H_

// ROOT
#include "TObject.h"  //For ClassDef
#include "TTimeStamp.h"

// STL
#include <iostream>
#include <map>
#include <string>

namespace ldmx {

/**
 * @class EventHeader
 * @brief Provides header information an event such as event number and
 * timestamp
 *
 * The evolution of the EventHeader object has been pretty slow since
 * the `*Parameter* members can be used to hold most additional information.
 * ROOT's serialization infrastructure does define a class version and so
 * we document the versions here.
 *
 * ## v1
 * This was the initial version of the EventHeader and should be considered
 * a "beta" version. It is not even internally consistent in that some changes
 * were made to the EventHeader and how it was serialized without modifying
 * this version number. It has largely been lost to the sands of time as
 * Framework has been moved into its own repository out of ldmx-sw.
 *
 * ## v2
 * This is the main version currently and has all of the ldmx-sw necessary
 * information except the number of tries it took to generate any given event.
 * This is what motivated the update to v3.
 *
 * ## v3
 * This version includes the number of tries it took to generate an event
 * (from Framework's point of view) and updates the `get*Parameter` functions
 * to be `const` so that they can be accessed from within analyzers.
 *
 * ### v3-v2 Interop
 * The interoperability of the v2 and v3 event headers was studied during
 * the merging process of the v3 updates. You can view the full details
 * on GitHub: https://github.com/LDMX-Software/Framework/pull/80.
 * In summary, reading v3 headers with v2 will print a warning message
 * and write a file that includes an _empty_ `tries_` subbranch. Reading
 * v2 headers with v3 will silently maintain the v2 structure (i.e. there
 * will be no `tries_` subbranch).
 */
class EventHeader {
 public:
  /**
   * Name of EventHeader branch
   */
  static const std::string BRANCH;

  /**
   * Class constructor.
   */
  EventHeader() = default;

  /**
   * Class destructor.
   */
  virtual ~EventHeader() = default;

  /**
   * Clear information from this object.
   *
   * @param[in] o ROOT-style Option (ignored)
   */
  void Clear(Option_t* o = "");

  /**
   * Print this object.
   * @param[in] o ROOT-style Option (ignored)
   */
  void Print(Option_t* o = "") const;

  /**
   * Return the event number.
   * @return The event number.
   */
  int getEventNumber() const { return eventNumber_; }

  /**
   * Return the run number.
   * @return The run number.
   */
  int getRun() const { return run_; }

  /**
   * Get the event's timestamp.
   * This currently returns second's since the epoch for simulated events.
   * @return The event's timestamp.
   * @note The returned object has a possible resolution of nanoseconds.
   */
  const TTimeStamp& getTimestamp() const { return timestamp_; }

  /**
   * Get the event weight (default of 1.0).
   * @return The event weight.
   */
  double getWeight() const { return weight_; }

  /**
   * Increment the number of events tried by one
   *
   * @note This modification function is used within
   * Framework during Production Mode (i.e. no input files).
   * A Producer also incrementing the number of tries during
   * Production Mode is undefined behavior.
   */
  void incrementTries() { tries_++; }

  /**
   * Get the number of events tried
   * @return the number of tries
   */
  int getTries() const { return tries_; }

  /**
   * Is this a real data event?
   * @return True if this is a real data event.
   */
  bool isRealData() const { return isRealData_; }
  
  /**
   * set whether this event is real or MC data
   * @param[in] yes True if this event is real data
   */
  void setRealData(bool yes = true) {
    isRealData_ = yes;
  }

  /**
   * Set the event number.
   * @note This is used within Framework during Production Mode.
   * It is untested but Producers can probably change the event number
   * without issue since the tracking of the number of events processed
   * is done separately within Framework.
   * @param eventNumber The event number.
   */
  void setEventNumber(int eventNumber) { this->eventNumber_ = eventNumber; }

  /**
   * Set the run number.
   * @note This is used within Framework during Production Mode.
   * It is untested but Producers can probably change the run number
   * without issue.
   * @param run The run number.
   */
  void setRun(int run) { this->run_ = run; }

  /**
   * Set the timestamp.
   * @note This is used within Framework during Production Mode.
   * It is untested but Producers can probably change the timestamp
   * without issue.
   * @param timestamp The timestamp.
   */
  void setTimestamp(const TTimeStamp& timestamp) {
    this->timestamp_ = timestamp;
  }

  /**
   * Set the event weight.
   * 
   * The event weight is by default 1 for all events. It is up to
   * a downstream producer to update the event weight if their procedure
   * demands it (for example, a simulation producer would copy its event
   * weight here).
   *
   * @param weight The event weight.
   */
  void setWeight(double weight) { this->weight_ = weight; }

  /**
   * Get an int parameter value.
   * @throw Exception if parameter does not exist
   * @param name The name of the parameter.
   * @return The parameter value.
   */
  int getIntParameter(const std::string& name) const;

  /**
   * Set an int parameter value.
   * @param name The name of the parameter.
   * @param value The value of the parameter.
   * @return The parameter value.
   */
  void setIntParameter(const std::string& name, int value) {
    intParameters_[name] = value;
  }

  /**
   * Get a float parameter value.
   * @throw Exception if parameter does not exist
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  float getFloatParameter(const std::string& name) const;

  /**
   * Set a float parameter value.
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  void setFloatParameter(const std::string& name, float value) {
    floatParameters_[name] = value;
  }

  /**
   * Get a string parameter value.
   * @throw Exception if parameter does not exist
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  std::string getStringParameter(const std::string& name) const;

  /**
   * Set a string parameter value.
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  void setStringParameter(const std::string& name, std::string value) {
    stringParameters_[name] = value;
  }

 protected:
  /**
   * The event number.
   */
  int eventNumber_{-1};

  /**
   * The run number.
   */
  int run_{-1};

  /**
   * The event timestamp
   */
  TTimeStamp timestamp_{0, 0};

  /**
   * The event weight.
   */
  double weight_{1.0};

  /**
   * The number of events that were begun before
   * this event was generated and accepted by event filtering.
   */
  int tries_{0};

  /**
   * Is this event real data?
   */
  bool isRealData_{false};

  /**
   * The int parameters.
   */
  std::map<std::string, int> intParameters_;

  /**
   * The float parameters.
   */
  std::map<std::string, float> floatParameters_;

  /**
   * The string parameters.
   */
  std::map<std::string, std::string> stringParameters_;

  /**
   * ROOT class definition.
   */
  ClassDef(EventHeader, 3);
};

}  // namespace ldmx

#endif /* EVENT_EVENTHEADER_H_ */
