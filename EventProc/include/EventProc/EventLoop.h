#ifndef EventProc_EventLoop_h
#define EventProc_EventLoop_h

#include "EventProc/EventProcessor.h"
#include "EventProc/EventSource.h"

using eventproc::EventProcessor;
using eventproc::EventSource;

namespace eventproc {

class EventLoop {

    public:

        EventLoop() : eventSource(nullptr) {;}

        virtual ~EventLoop() {;}

        void initialize();

        void run(int nEvents);

        void finish();

        void addEventProcessor(EventProcessor* eventProcessor) {
            processors.push_back(eventProcessor);
        }

        void setEventSource(EventSource* eventSource) {
            this->eventSource = eventSource;
        }

    private:

        std::vector<EventProcessor*> processors;
        EventSource* eventSource;
};

}

#endif
