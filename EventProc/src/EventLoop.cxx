#include "EventProc/EventLoop.h"

// STL
#include <stdexcept>

namespace eventproc {

void EventLoop::initialize() {
    for (EventProcessor* processor : processors_) {
        processor->setEvent(eventSource_->getEvent()); 
        processor->initialize(); 
    }
       
    /*for (std::vector<EventProcessor*>::iterator procIt = processors_.begin();
            procIt != processors_.end(); procIt++) {
        (*procIt)->setEvent(eventSource_->getEvent());
        (*procIt)->initialize();
    }*/
}

void EventLoop::run(int nEvents) {
    int nProcessed = 0;
    while (eventSource_->nextEvent()) {
        std::cout << "Event: " << nProcessed << std::endl;
        /*for (std::vector<EventProcessor*>::iterator procIt = processors_.begin();
                procIt != processors_.end(); procIt++) {
            (*procIt)->execute();
        }*/
        ++nProcessed;
        if (nEvents > 0 && nProcessed >= nEvents) {
            break;
        }
    }
    std::cout << "EventLoop: Finished processing " << nProcessed << " events out of "
            << nEvents << " requested." << std::endl;
}

void EventLoop::finish() {
    for (EventProcessor* processor : processors_) {
       processor->finish(); 
    }
    /*
    for (std::vector<EventProcessor*>::iterator procIt = processors_.begin();
                    procIt != processors_.end(); procIt++) {
        (*procIt)->finish();
    }*/
}

void EventLoop::setEventSource(event::EventFile* eventSource) { 
    eventSource_ = eventSource;
}

}
