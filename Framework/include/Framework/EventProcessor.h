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
    
    static void declare(const char* classname, int classtype,EventProcessorMaker*);
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

#define DECLARE_PRODUCER(CLASS) extern "C" { ldmxsw::EventProcessor* ldmxsw_make_ ## CLASS (const std::string& name, const ldmxsw::Process& process) { return new CLASS(name,process); } } __attribute__((constructor)) static void ldmxsw_declare_ ## CLASS() { ldmxsw::EventProcessor::declare(#CLASS,::ldmxsw::Producer::CLASSTYPE,&ldmxsw_make_ ## CLASS ); }

#define DECLARE_ANALYZER(CLASS) extern "C" { ldmxsw::EventProcessor* ldmxsw_make_ ## CLASS (const std::string& name, const ldmxsw::Process& process) { return new CLASS(name,process); } } __attribute__((constructor)) static void ldmxsw_declare_ ## CLASS() { ldmxsw::EventProcessor::declare(#CLASS,::ldmxsw::Analyzer::CLASSTYPE,&ldmxsw_make_ ## CLASS ); }



#endif // ldmxsw_Framework_EventProcessor_h_included
