/**
 * @file ConditionsObjectProvider.h
 * @brief Base class for provider of conditions information like pedestals, gains, electronics maps, etc
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_CONDITIONSOBJECTPROVIDER_H_
#define FRAMEWORK_CONDITIONSOBJECTPROVIDER_H_

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Framework/Exception.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 
#include "Framework/Logger.h"
#include "Framework/ConditionsIOV.h"
#include "Framework/ConditionsObject.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>

namespace ldmx {

class Process;
class ConditionsObjectProvider;
class EventHeader;
class RunHeader;

/** Typedef for PluginFactory use. */
typedef ConditionsObjectProvider* 
ConditionsObjectProviderMaker(const std::string& objname, const std::string& tagname, const Parameters& params, Process& process);

/**
 * @class ConditionsObjectProvider
 * @brief Base class for all providers of conditions objects
 */
class ConditionsObjectProvider {

 public:

  /** Constant used to types by the PluginFactory */
  static const int CLASSTYPE{10};
        
  /**
   * Class constructor.
   * @param name Name for this instance of the class.
   * @param tagName The tag for the database entry (should not include whitespace)
   * @param process The Process class associated with ConditionsObjectProvider, provided by the framework.
   *
   * @note The name provided to this function should not be
   * the class name, but rather a logical label for this instance of
   * the class, as more than one copy of a given class can be loaded
   * into a Process with different parameters.  Names should not include
   * whitespace or special characters.
   */
  ConditionsObjectProvider(const std::string& objname, const std::string& tagname, const Parameters& parameters, Process& process);

  /**
   * Class destructor.
   */
  virtual ~ConditionsObjectProvider() {;}

  /**
   * Pure virtual getCondition function.
   * Must be implemented by any Conditions providers.
   */
  virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const EventHeader& context) = 0;

  /**
   * Called by conditions system when done with a conditions object, appropriate point for cleanup.
   * @note Default behavior is to delete the object!
   */
  virtual void releaseConditionsObject(const ConditionsObject* co) {
    delete co;
  }
    
  /**
   * Callback for the ConditionsObjectProvider to take any necessary
   * action when the processing of events starts.
   */
  virtual void onProcessStart() {
  }

  /**
   * Callback for the ConditionsObjectProvider to take any necessary
   * action when the processing of events finishes, such as closing
   * database connections.
   */
  virtual void onProcessEnd() {
  }

  /**
   * Callback for the ConditionsObjectProvider to take any necessary
   * action when the processing of events starts for a given run.
   */
  virtual void onNewRun(RunHeader&) {
  }

  /**
   * Get the list of conditions objects available from this provider.
   */
  const std::string& getConditionObjectName() const { return objectName_; }
        
  /**
   * Internal function which is part of the PluginFactory machinery.
   * @param classname The class name of the processor.
   */
  static void declare(const std::string& classname, ConditionsObjectProviderMaker*);

  /** 
   * Access the tag name
   */
  const std::string& getTagName() const { return tagname_; }
    
 protected:

  /** Request another condition needed to construct this condition */
  std::pair<const ConditionsObject*,ConditionsIOV> requestParentCondition(const std::string& name, const EventHeader& context);
  
  /// The logger for this ConditionsObjectProvider
  logging::logger theLog_;

  /** Get the process handle */
  const Process& process() const { return process_; }

 private:

  /** Handle to the Process. */
  Process& process_;

  /** The name of the object provided by this provider. */
  std::string objectName_;

  /** The tag name for the ConditionsObjectProvider. */
  std::string tagname_;

};

}

/**
 * @def DECLARE_CONDITIONS_PROVIDER(CLASS)
 * @param CLASS The name of the class to register, which must not be in a namespace.  If the class is in a namespace, use DECLARE_CONDITIONS_PROVIDER_NS()
 * @brief Macro which allows the framework to construct a producer given its name during configuration.
 * @attention Every Producer class must call this macro or DECLARE_CONDITIONS_PROVIDER_NS() in the associated implementation (.cxx) file.
 */
#define DECLARE_CONDITIONS_PROVIDER(CLASS) ldmx::ConditionsObjectProvider*  CLASS ## _ldmx_make (const std::string& name, const std::string& tagname, const ldmx::Parameters& params, ldmx::Process& process) { return new CLASS(name,tagname,params,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmx_declare() { ldmx::ConditionsObjectProvider::declare(#CLASS,& CLASS ## _ldmx_make ); }

/**
 * @def DECLARE_CONDITIONS_PROVIDER_NS(NS,CLASS)
 * @param NS The full namespace specification for the Producer
 * @param CLASS The name of the class to register
 * @brief Macro which allows the framework to construct a producer given its name during configuration.
 * @attention Every Producer class must call this macro or DECLARE_CONDITIONS_PROVIDER() in the associated implementation (.cxx) file.
 */
#define DECLARE_CONDITIONS_PROVIDER_NS(NS, CLASS) ldmx::ConditionsObjectProvider*  CLASS ## _ldmx_make (const std::string& name, const std::string& tagname, const ldmx::Parameters& params, ldmx::Process& process) { return new NS::CLASS(name,tagname,params,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmx_declare() { ldmx::ConditionsObjectProvider::declare(std::string(#NS)+"::"+std::string(#CLASS),& CLASS ## _ldmx_make ); }

#endif 
