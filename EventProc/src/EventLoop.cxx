#include "EventProc/EventLoop.h"

// STL
#include <stdexcept>

namespace eventproc {

void EventLoop::initialize() {
    for (EventProcessor* processor : processors_) {
        processor->setEvent(eventSource_->getEvent()); 
        processor->initialize(); 
    }
}

void EventLoop::run(int nEvents) {
    int nProcessed = 0;
    while (eventSource_->nextEvent()) {
        std::cout << "Event: " << nProcessed << std::endl;
        for (EventProcessor* processor : processors_) {
            processor->execute(); 
        }
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
}

void EventLoop::setEventSource(event::EventFile* eventSource) { 
    eventSource_ = eventSource;
}

}
