#ifndef ldmxsw_Framework_Process_h_included
#define ldmxsw_Framework_Process_h_included 1

#include "Framework/Exception.h"
#include <vector>

namespace ldmxsw {

  class EventProcessor;
  class EventFile;
  class EventImpl;
  
   /** Class which represents the process under execution.
   */
  class Process {
  public:
    Process(const std::string& passname);

    const std::string& passname() const { return passname_; }
    
    void addToSequence(EventProcessor* mod);
    void addFileToProcess(const std::string& filename);
    void addDropKeepRule(const std::string& rule);
    void setOutputFileTemplate(const std::string& filenameOut);

    void setRunNumber(int run) { runForGeneration_=run; }
    void run(int events=-1);

  private:
    std::string passname_;
    std::vector<EventProcessor*> sequence_;
    std::vector<std::string> inputFiles_;
    std::string outputFileNameRule_;
    std::vector<std::string> dropKeepRules_;

    int runForGeneration_{1};
  };

}

#endif // ldmxsw_Framework_Process_h_included
