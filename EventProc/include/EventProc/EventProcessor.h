#ifndef EVENTPROC_EVENTPROCESSOR_H_
#define EVENTPROC_EVENTPROCESSOR_H_

#include "Event/Event.h"

using event::Event;

namespace eventproc {

class EventProcessor {

    public:

        EventProcessor() : event_(nullptr) {;}

        virtual ~EventProcessor() {;}

        virtual void initialize() = 0;

        virtual void execute() = 0;

        virtual void finish() = 0;

        void setEvent(Event* anEvent) {
            this->event_ = anEvent;
        }

        Event* getEvent() {
            return event_;
        }

    private:

        Event* event_;
};

}

#endif
