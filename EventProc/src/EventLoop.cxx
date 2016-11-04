#include "EventProc/EventLoop.h"

// STL
#include <stdexcept>

namespace eventproc {

void EventLoop::initialize() {
    for (std::vector<EventProcessor*>::iterator procIt = processors_.begin();
            procIt != processors_.end(); procIt++) {
        (*procIt)->setEvent(eventSource_->getEvent());
        (*procIt)->initialize();
    }
}

void EventLoop::run(int nEvents) {
    int nProcessed = 0;
    while (eventSource_->readNextEvent()) {
        for (std::vector<EventProcessor*>::iterator procIt = processors_.begin();
                procIt != processors_.end(); procIt++) {
            (*procIt)->execute();
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
    for (std::vector<EventProcessor*>::iterator procIt = processors_.begin();
                    procIt != processors_.end(); procIt++) {
        (*procIt)->finish();
    }
}

}
