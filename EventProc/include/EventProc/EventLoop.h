/**
 * @file EventLoop.h
 * @brief Class that implements a basic event processing loop
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_EVENTLOOP_H_
#define EVENTPROC_EVENTLOOP_H_

#include "Event/EventFile.h"
#include "Event/EventImpl.h"
#include "EventProc/EventProcessor.h"
#include "EventProc/EventSource.h"

using eventproc::EventProcessor;
using eventproc::EventSource;

namespace eventproc {

/**
 * @class EventLoop
 * @brief Basic event processing loop
 *
 * @note
 * This class uses an EventSource to supply records to a list of registered
 * EventProcessor objects.
 */
class EventLoop {

    public:

        /**
         * Class constructor.
         */
        EventLoop() : eventSource_(nullptr) {;}

        /**
         * Class destructor.
         */
        virtual ~EventLoop() {;}

        /**
         * Activates <i>initialize()</i> methods of registered EventProcessor objects.
         */
        void initialize();

        /**
         * Process a number of events.
         * @param nEvents The number of events to process (-1 for unlimited).
         */
        void run(int nEvents);

        /**
         * Activate <i>finish()</i> methods of registered EventProcessor objects.
         */
        void finish();

        /**
         * Register an EventProcessor.
         * @param eventProcessor The EventProcessor to register.
         */
        void addEventProcessor(EventProcessor* eventProcessor) {
            processors_.push_back(eventProcessor);
        }

        /**
         * Set the event source.
         * @param eventSource The EventSource supplying events to the loop.
         */
        void setEventSource(event::EventFile* eventSource);

    private:

        /**
         * The list of registered EventProcessor objects.
         */
        std::vector<EventProcessor*> processors_;

        /**
         * The event source supplying events to the loop.
         */
        event::EventFile* eventSource_{nullptr};

};

}

#endif
