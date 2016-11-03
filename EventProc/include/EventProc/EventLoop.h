#ifndef EVENTPROC_EVENTLOOP_H_
#define EVENTPROC_EVENTLOOP_H_

#include "EventProc/EventProcessor.h"
#include "EventProc/EventSource.h"

using eventproc::EventProcessor;
using eventproc::EventSource;

namespace eventproc {

class EventLoop {

    public:

        EventLoop() : eventSource_(nullptr) {;}

        virtual ~EventLoop() {;}

        void initialize();

        void run(int nEvents);

        void finish();

        void addEventProcessor(EventProcessor* eventProcessor) {
            processors_.push_back(eventProcessor);
        }

        void setEventSource(EventSource* eventSource) {
            this->eventSource_ = eventSource;
        }

    private:

        std::vector<EventProcessor*> processors_;
        EventSource* eventSource_;
};

}

#endif
