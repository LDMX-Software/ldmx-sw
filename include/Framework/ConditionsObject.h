/**
 * @file ConditionsObject.h
 * @brief Base class for conditions information like pedestals, gains, electronics maps, etc
 * @author Jeremy Mans, University of Minnesota
 */
#ifndef FRAMEWORK_CONDITIONSOBJECT_H_
#define FRAMEWORK_CONDITIONSOBJECT_H_

#include <string>

namespace ldmx {

/**
 * @class ConditionsObject
 * @brief Base class for all conditions objects, very simple
 */
class ConditionsObject {
 public:
  /**
   * Class constructor
   */
  ConditionsObject(const std::string& name) : name_(name) { }
        
  /**
   * Destructor
   */
  virtual ~ConditionsObject() { }
        
  /**
   * Get the name of this object
   */
  std::string getName() const { return name_; }
        
 private:
  /** 
   * Name of the object
   */
  std::string name_;
};

} // namespace ldmx

#endif
