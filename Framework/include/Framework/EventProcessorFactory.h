#ifndef ldmxsw_Framework_EventProcessorFactory_h_included
#define ldmxsw_Framework_EventProcessorFactory_h_included 1

#include <map>
#include <set>
#include <vector>
#include "Framework/EventProcessor.h"

namespace ldmxsw {

  /** Singleton module factory
   */
  class EventProcessorFactory {
  public:
    static EventProcessorFactory& getInstance() { return theFactory_; }

    void registerEventProcessor(const std::string& classname, int classtype, EventProcessorMaker* maker);
    std::vector<std::string> getEventProcessorClasses() const;
    int getEventProcessorClasstype(const std::string&) const;
    EventProcessor* createEventProcessor(const std::string& classname, const std::string& moduleInstanceName, Process& process);
    void loadLibrary(const std::string& libname);

  private:
    EventProcessorFactory();

    struct EventProcessorInfo {
      std::string classname;
      int classtype;
      EventProcessorMaker* maker;
    };
    std::map<std::string,EventProcessorInfo> moduleInfo_;
    std::set<std::string> librariesLoaded_;

    static EventProcessorFactory theFactory_;
  };

}

#endif // ldmxsw_Framework_EventProcessorFactory_h_included
