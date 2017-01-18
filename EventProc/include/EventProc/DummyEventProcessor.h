/**
 * @file DummyEventProcessor.h
 * @brief Class defining a dummy implementation of EventProcessor
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_DUMMYEVENTPROCESSOR_H_
#define EVENTPROC_DUMMYEVENTPROCESSOR_H_

#include "Event/EventConstants.h"
#include "EventProc/EventProcessor.h"
#include <iostream>

namespace eventproc {

/**
 * @class DummyEventProcessor
 * @brief Dummy implementation of EventProcessor
 */
class DummyEventProcessor : public EventProcessor {

    public:

        void initialize() {
            std::cout << "[ DummyEventProcessor ] : Initializing" << std::endl;
        }

        void execute() {
            std::cout << "[ DummyEventProcessor ] : Processing event "
                    << event_->getEventHeader("sim")->getEventNumber() << std::endl;
            ++nProcessed_;
        }

        void finish() {
            std::cout << "[ DummyEventProcessor ] : Finished processing "
                    << nProcessed_ << " events." << std::endl;
        }

    private:

        int nProcessed_{0};
};

}

#endif
