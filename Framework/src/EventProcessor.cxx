#include "Framework/Process.h"
#include "Framework/ParameterSet.h"
#include "Framework/EventProcessor.h"
#include "Framework/EventProcessorFactory.h"

namespace ldmxsw {

EventProcessor::EventProcessor(const std::string& name, const Process& process) :
        process_ { process }, name_ { name } {
}

void EventProcessor::declare(const std::string& classname, int classtype, EventProcessorMaker* maker) {
    EventProcessorFactory::getInstance().registerEventProcessor(classname, classtype, maker);
}

Producer::Producer(const std::string& name, const Process& process) :
        EventProcessor(name, process) {
}
Analyzer::Analyzer(const std::string& name, const Process& process) :
        EventProcessor(name, process) {
}

}
