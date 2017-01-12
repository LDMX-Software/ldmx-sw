#include "TObject.h"

#ifndef EVENT_EVENTHEADER_H_
#define EVENT_EVENTHEADER_H_

namespace event {

class EventHeader : public TObject {

    public:

        /**
         * Class constructor.
         */
        EventHeader() {;}

        /**
         * Class destructor.
         */
        virtual ~EventHeader() {;}

        /**
         * Clear information from this object.
         */
        void Clear(Option_t* = "") {
            eventNumber_ = -1;
            run_= -1;
            timestamp_ = -1;
            weight_ = 1.0;
        }

        void Print(Option_t* = "")  const {
            std::cout << "EventHeader {"
                    << " eventNumber: " << eventNumber_
                    << ", run: " << run_
                    << ", timestamp: " << timestamp_
                    << ", weight: " << weight_
                    << " }"
                    << std::endl;
        }

        void Copy(TObject& o) const {
            std::cout << "EventHeader::Copy" << std::endl;
            ((EventHeader&)o) = *this;
        }

        /**
         * Return the event number.
         */
        int getEventNumber() const { return eventNumber_; }

        /**
         * Return the run number.
         */
        int getRun() const { return run_; }

        /**
         * Get the event's timestamp, currently in seconds.
         */
        int getTimestamp() const { return timestamp_; }

        /**
         * Get the event weight.
         */
        double getWeight() const { return weight_; }

        /**
         * Set the event number.
         * @param eventNumber The event number.
         */
        void setEventNumber(int eventNumber) { this->eventNumber_ = eventNumber; }

        /**
         * Set the run number.
         * @param run The run number.
         */
        void setRun(int run) { this->run_ = run; }

        /**
         * Set the timestamp.
         * @param timestamp The timestamp.
         */
        void setTimestamp(int timestamp) { this->timestamp_ = timestamp; }

        /**
         * Set the event weight.
         * @param weight The event weight.
         */
        void setWeight(double weight) { this->weight_ = weight; }

    protected:

        /**
         * The event number.
         */
        int eventNumber_{-1};

        /**
         * The run number.
         */
        int run_{-1};

        /**
         * The event timestamp, in seconds.
         */
        int timestamp_{-1};

        /**
         * The event weight.
         */
        double weight_{1.0};

    /**
     * ROOT class definition.
     */
    ClassDef(EventHeader, 1);
};

}

#endif /* EVENT_EVENTHEADER_H_ */
