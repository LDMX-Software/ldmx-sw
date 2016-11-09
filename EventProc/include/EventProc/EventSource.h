#ifndef EVENTPROC_EVENTSOURCE_H_
#define EVENTPROC_EVENTSOURCE_H_

#include "Event/Event.h"

using event::Event;

namespace eventproc {

class EventSource {

    public:

        EventSource(Event* anEvent) : _event(anEvent) {;}

        virtual ~EventSource() {;}

        virtual bool readNextEvent() = 0;

        Event* getEvent() {
            return _event;
        }

    protected:

        Event* _event;
};

}

#endif
