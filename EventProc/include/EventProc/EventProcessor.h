/**
 * @file EventProcessor.h
 * @brief Class that defines an abstract interface for sequential event processing
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_EVENTPROCESSOR_H_
#define EVENTPROC_EVENTPROCESSOR_H_

#include "Event/EventImpl.h"

/**
 * @namespace eventproc
 * @brief Event processing classes for reconstruction and analysis
 */
namespace eventproc {

/**
 * @class EventProcessor
 * @brief Abstract interface to be implemented for sequential event processing
 *
 * @note
 * Instances of this class can be registered with an EventLoop to perform
 * sequential event processing.
 */
class EventProcessor {

    public:

        /**
         * Class constructor.
         */
        EventProcessor() : event_(nullptr) {;}

        /**
         * Class destructor.
         */
        virtual ~EventProcessor() {;}

        /**
         * Perform initialization at start of job.
         */
        virtual void initialize() = 0;

        /**
         * Execute this processor on the current event.
         */
        virtual void execute() = 0;

        /**
         * Perform cleanup at end of job.
         */
        virtual void finish() = 0;

        /**
         * Set the current event.
         * Since the EventLoop uses Event as a buffer filled from a tree,
         * this is only called once at the beginning of the job.
         * @param anEvent The current event.
         */
        void setEvent(event::EventImpl* anEvent) {
            this->event_ = anEvent;
        }

        /**
         * Get the current event.
         * @return The current event.
         */
        event::EventImpl* getEvent() {
            return event_;
        }

    private:

        /**
         * Pointer to the event buffer.
         */
        event::EventImpl* event_;
};

}

#endif
