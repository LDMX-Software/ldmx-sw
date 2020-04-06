#include "Framework/EventProcessor.h"

// LDMX
#include "Framework/Process.h"
#include "Framework/EventProcessorFactory.h"
#include "TDirectory.h"
#include "Event/RunHeader.h"

namespace ldmx {

    EventProcessor::EventProcessor(const std::string& name, Process& process) :
        process_ (process ), name_ { name } {

            theLog_ = logging::makeLogger( name_ );
    }

    void EventProcessor::declare(const std::string& classname, int classtype,EventProcessorMaker* maker) {
        EventProcessorFactory::getInstance().registerEventProcessor(classname, classtype, maker);
    }

    void EventProcessor::setStorageHint(ldmx::StorageControlHint hint, const std::string& purposeString) {
        process_.getStorageController().addHint(name_,hint,purposeString);
    }
  
    TDirectory* EventProcessor::getHistoDirectory() {
        if (!histoDir_) {
            histoDir_=process_.makeHistoDirectory(name_);
        }
        histoDir_->cd(); // make this the current directory
        return histoDir_;
    }

    Producer::Producer(const std::string& name, Process& process) : EventProcessor(name,process) {}

    Analyzer::Analyzer(const std::string& name, Process& process) : EventProcessor(name,process) {}
}
