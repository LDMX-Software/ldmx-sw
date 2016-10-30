#ifndef EVENTPROC_EVENTSOURCE_H_
#define EVENTPROC_EVENTSOURCE_H_

#include "Event/Event.h"

using event::Event;

namespace eventproc {

class EventSource {

    public:

        EventSource(Event* event) : event(event) {;}

        virtual ~EventSource() {;}

        virtual bool readNextEvent() = 0;

        Event* getEvent() {
            return event;
        }

    protected:

        Event* event;
};

}

#endif
