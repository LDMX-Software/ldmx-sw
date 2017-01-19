#ifndef LDMXSW_FRAMEWORK_PROCESS_H_
#define LDMXSW_FRAMEWORK_PROCESS_H_

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
        void setOutputFileName(const std::string& filenameOut);
        void addOutputFileName(const std::string& filenameOut);
        
        void setRunNumber(int run) { runForGeneration_=run; }
        void setEventLimit(int limit) { eventLimit_=limit; }
        void run();

    private:

        std::string passname_;
        int eventLimit_;
        std::vector<EventProcessor*> sequence_;
        std::vector<std::string> inputFiles_;
        std::vector<std::string> outputFiles_;
        std::vector<std::string> dropKeepRules_;
        
        int runForGeneration_{1};
};
    
}

#endif
