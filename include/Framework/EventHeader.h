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
  EventHeader() {}

  /**
   * Class destructor.
   */
  virtual ~EventHeader() {}

  /**
   * Clear information from this object.
   */
  void Clear(Option_t* = "") {
    eventNumber_ = -1;
    run_ = -1;
    timestamp_ = TTimeStamp(0, 0);
    weight_ = 1.0;
    tries_ = 0;
    isRealData_ = false;
    intParameters_.clear();
    floatParameters_.clear();
    stringParameters_.clear();
  }

  /**
   * Print this object.
   */
  void Print(Option_t* = "") const {
    std::cout << "EventHeader {"
              << " eventNumber: " << eventNumber_ << ", run: " << run_
              << ", timestamp: " << timestamp_ << ", weight: " << weight_
              << ", tries: " << tries_;
    if (isRealData_)
      std::cout << ", DATA";
    else
      std::cout << ", MC";
    std::cout << " }" << std::endl;
  }

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
   * @param eventNumber The event number.
   */
  void setEventNumber(int eventNumber) { this->eventNumber_ = eventNumber; }

  /**
   * Set the run number.
   * @param run The run number.
   */
  void setRun(int run) { this->run_ = run; }

  /**
   * Set the timestamp.
   * @param timestamp The timestamp.
   */
  void setTimestamp(const TTimeStamp& timestamp) {
    this->timestamp_ = timestamp;
  }

  /**
   * Set the event weight.
   * @param weight The event weight.
   */
  void setWeight(double weight) { this->weight_ = weight; }

  /**
   * Get an int parameter value.
   * @param name The name of the parameter.
   * @return The parameter value.
   */
  int getIntParameter(const std::string& name) { return intParameters_[name]; }

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
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  float getFloatParameter(const std::string& name) {
    return floatParameters_[name];
  }

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
   * @param name The name of the parameter.
   * @return value The parameter value.
   */
  std::string getStringParameter(const std::string& name) {
    return stringParameters_[name];
  }

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
