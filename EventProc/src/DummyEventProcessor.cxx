#include "EventProc/DummyEventProcessor.h"

#include "Event/EventConstants.h"
#include <iostream>

using event::EventConstants;

namespace eventproc {

void DummyEventProcessor::initialize() {
    std::cout << "[ DummyEventProcessor ] : Initializing" << std::endl;
}

void DummyEventProcessor::execute() {
    std::cout << "[ DummyEventProcessor ] : Processing event "
            << event_->getEventHeader()->getEventNumber() << std::endl;
    ++nProcessed_;
}

void DummyEventProcessor::finish() {
    std::cout << "DummyEventProcessor: Finished processing "
            << nProcessed_ << " events." << std::endl;
}

}
