#ifndef EventProc_EventSource_h
#define EventProc_EventSource_h

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
