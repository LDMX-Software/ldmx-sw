/**
 * @file ConditionsObject.h
 * @brief Base class for conditions information like pedestals, gains, electronics maps, etc
 * @author Jeremy Mans, University of Minnesota
 */
#ifndef FRAMEWORK_CONDITIONSOBJECT_H_
#define FRAMEWORK_CONDITIONSOBJECT_H_

namespace ldmx {

  class ConditionsObjectProvider;

   /**
     * @class ConditionsObject
     * @brief Base class for all conditions objects, very simple
     */
  class ConditionsObject {
  public:
    /**
     * Class constructor
     */
    ConditionsObject(const std::string& name, const ConditionsObjectProvider& provider) : name_(name), provider_{provider} {
    }

    /**
     * Destructor
     */
    virtual ~ConditionsObject() { }

    /**
     * Get the name of this object
     */
    std::string getName() const { return name_; }
    
    /** 
     * Get access to the owner
     */
    const ConditionsObjectProvider& provider() { return provider_; }
  private:
    /** 
     * Name of the object
     */
    std::string name_;

    /** 
     * Source of this object
     */
    const ConditionsObjectProvider& provider_;
  };
}

#endif
