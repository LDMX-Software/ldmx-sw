#ifndef EVENTPROC_EVENTPROCESSOR_H_
#define EVENTPROC_EVENTPROCESSOR_H_

#include "Event/Event.h"

using event::Event;

namespace eventproc {

class EventProcessor {

    public:

        EventProcessor() : event(nullptr) {;}

        virtual ~EventProcessor() {;}

        virtual void initialize() = 0;

        virtual void execute() = 0;

        virtual void finish() = 0;

        void setEvent(Event* event) {
            this->event = event;
        }

        Event* getEvent() {
            return event;
        }

    private:

        Event* event;
};

}

#endif
