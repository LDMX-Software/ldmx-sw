#ifndef ldmxsw_Framework_EventProcessor_h_included
#define ldmxsw_Framework_EventProcessor_h_included 1

#include "Framework/Exception.h"

#include <map>

namespace event {
  class Event;
}

namespace ldmxsw {

  class ParameterSet;
  class Process;
  class EventProcessor;
  
  typedef EventProcessor* EventProcessorMaker(const std::string& name, const Process& process);

  /** Base class for a module
   */
  class EventProcessor {
  public:
    EventProcessor(const std::string& name, const Process& process);

    virtual void configure(const ParameterSet&) { }
    virtual void onFileOpen() { }
    virtual void onFileClose() { }
    virtual void onProcessStart() { }
    virtual void onProcessEnd() { }
    
      static void declare(const std::string& classname, int classtype,EventProcessorMaker*);
  protected:
    const Process& process_;    
  private:
    std::string name_;
  };

  /** Base class for a module which produces a data product (thus needs a mutable copy of the event)
   */
  class Producer : public EventProcessor {
  public:
    static const int CLASSTYPE{1};
    Producer(const std::string& name, const Process& process);

    virtual void produce(event::Event& event) = 0;    
  };


  /** Base class for a module which does not produce a data product (thus needs only a constant copy of the event)
   */
  class Analyzer : public EventProcessor {
  public:
    static const int CLASSTYPE{2};
    Analyzer(const std::string& name, const Process& process);

    virtual void analyze(const event::Event& event) = 0;    

  };

}

/** @def DECLARE_PRODUCER(CLASS)
    @param CLASS The name of the class to register, which must not be in a namespace.  If the class is in a namespace, use DECLARE_PRODUCER_NS()
    @brief Macro which allows the framework to construct a producer given its name during configuration.  
    @attention Every Producer class must call this macro or DECLARE_PRODUCER_NS() in the associated implementation (.cxx) file.
*/
#define DECLARE_PRODUCER(CLASS) ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(#CLASS,::ldmxsw::Producer::CLASSTYPE,& CLASS ## _ldmxsw_make ); }

/** @def DECLARE_ANALYZER(CLASS)
    @param CLASS The name of the class to register, which must not be in a namespace.  If the class is in a namespace, use DECLARE_PRODUCER_NS()
    @brief Macro which allows the framework to construct an analyzer given its name during configuration.  
    @attention Every Analyzer class must call this macro or DECLARE_ANALYZER_NS() in the associated implementation (.cxx) file.
*/
#define DECLARE_ANALYZER(CLASS)  ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(#CLASS,::ldmxsw::Analyzer::CLASSTYPE,& CLASS ## _ldmxsw_make); }


/** @def DECLARE_PRODUCER_NS(NS,CLASS)
    @param NS The full namespace specification for the Producer
    @param CLASS The name of the class to register
    @brief Macro which allows the framework to construct a producer given its name during configuration.  
    @attention Every Producer class must call this macro or DECLARE_PRODUCER() in the associated implementation (.cxx) file.
*/
#define DECLARE_PRODUCER_NS(NS,CLASS) ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new NS::CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(std::string(#NS)+"::"+std::string(#CLASS),::ldmxsw::Producer::CLASSTYPE,& CLASS ## _ldmxsw_make ); }

/** @def DECLARE_ANALYZER_NS(NS,CLASS)
    @param NS The full namespace specification for the Analyzer
    @param CLASS The name of the class to register
    @brief Macro which allows the framework to construct an analyzer given its name during configuration.  
    @attention Every Analyzer class must call this macro or DECLARE_ANALYZER() in the associated implementation (.cxx) file.
*/
#define DECLARE_ANALYZER_NS(NS,CLASS)  ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new NS::CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(std::string(#NS)+"::"+std::string(#CLASS),::ldmxsw::Analyzer::CLASSTYPE,& CLASS ## _ldmxsw_make); }



#endif // ldmxsw_Framework_EventProcessor_h_included
