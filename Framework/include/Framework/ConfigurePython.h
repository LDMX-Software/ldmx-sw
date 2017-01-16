#ifndef ldmxsw_Framework_ConfigurePython_h_included
#define ldmxsw_Framework_ConfigurePython_h_included 1

#include <string>
#include <vector>
#include "Framework/ParameterSet.h"

namespace ldmxsw {

  class Process;
  
  class ConfigurePython {
  public:
    ConfigurePython(const std::string& pythonScript);
    ~ConfigurePython();
    
    Process* makeProcess();

    int eventLimit() const { return eventLimit_; }
  private:
    
    std::string passname_;
    int eventLimit_,run_;
    std::vector<std::string> inputFiles_, keepRules_;
    
    struct ProcessorInfo {
      std::string classname_;
      std::string instancename_;
      ParameterSet params_;
    };
    std::vector<ProcessorInfo> sequence_;
    
  };
  
}

#endif // ldmxsw_Framework_ConfigurePython_h_included 1
 
