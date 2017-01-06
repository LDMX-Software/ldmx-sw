/**
 * @file EventSource.h
 * @brief Class that provides an abstract interface for supplying events to an event loop
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_EVENTSOURCE_H_
#define EVENTPROC_EVENTSOURCE_H_

#include "Event/Event.h"

using event::Event;

namespace eventproc {

/**
 * @class EventSource
 * @brief Supplies events for use by the EventLoop
 */
class EventSource {

    public:

        /**
         * Class constructor.
         */
        EventSource(Event* anEvent) : _event(anEvent) {;}

        /**
         * Class destructor.
         */
        virtual ~EventSource() {;}

        /**
         * Read the next event.
         * @return True if event was read successfully.
         */
        virtual bool readNextEvent() = 0;

        /**
         * Get the current event.
         * @return The current event.
         */
        Event* getEvent() {
            return _event;
        }

    protected:

        /**
         * The current event.
         */
        Event* _event;
};

}

#endif
