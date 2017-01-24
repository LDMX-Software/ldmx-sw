#include "Framework/Process.h"
#include "Framework/ParameterSet.h"
#include "Framework/EventProcessor.h"
#include "Framework/EventProcessorFactory.h"
#include "TDirectory.h"

namespace ldmxsw {

  
    EventProcessor::EventProcessor(const std::string& name, Process& process) : process_{process},name_{name} {
    }

    void EventProcessor::declare(const std::string& classname, int classtype,EventProcessorMaker* maker) {
	EventProcessorFactory::getInstance().registerEventProcessor(classname,classtype, maker);
    }

    TDirectory* EventProcessor::getHistoDirectory() {
	if (!histoDir_) {
	    histoDir_=process_.makeHistoDirectory(name_);
	}
	histoDir_->cd(); // make this the current directory
	return histoDir_;
    }

    Producer::Producer(const std::string& name, Process& process) : EventProcessor(name,process) { }
    Analyzer::Analyzer(const std::string& name, Process& process) : EventProcessor(name,process) { }
  
}
