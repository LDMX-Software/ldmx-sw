/**
 * @file EventHeader.h
 * @brief Class that provides header information about an event such as event number and timestamp
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_EVENTHEADER_H_
#define EVENT_EVENTHEADER_H_

// ROOT
#include "TObject.h"
#include "TTimeStamp.h"

// STL
#include <iostream>

namespace ldmx {

/**
 * @class EventHeader
 * @brief Provides header information an event such as event number and timestamp
 */
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
            run_ = -1;
            timestamp_ = TTimeStamp(0, 0);
            weight_ = 1.0;
            isRealData_ = false;
        }

        /**
         * Print this object.
         */
        void Print(Option_t* = "") const {
            std::cout << "EventHeader {" << " eventNumber: " << eventNumber_ << ", run: "
                    << run_ << ", timestamp: " << timestamp_ << ", weight: " << weight_;
            if (isRealData_)
                std::cout << ", DATA";
            else
                std::cout << ", MC";
            std::cout << " }" << std::endl;
        }

        /**
         * Copy this object.
         * @param o The target object.
         */
        void Copy(TObject& o) const {
            ((EventHeader&) o) = *this;
        }

        /**
         * Return the event number.
         * @return The event number.
         */
        int getEventNumber() const {
            return eventNumber_;
        }

        /**
         * Return the run number.
         * @return The run number.
         */
        int getRun() const {
            return run_;
        }

        /**
         * Get the event's timestamp.
         * This currently returns second's since the epoch for simulated events.
         * @return The event's timestamp.
         * @note The returned object has a possible resolution of nanoseconds.
         */
        const TTimeStamp& getTimestamp() const {
            return timestamp_;
        }

        /**
         * Get the event weight (default of 1.0).
         * @return The event weight.
         */
        double getWeight() const {
            return weight_;
        }

        /**
         * Get the event seeds (default is empty vector).
         * @return The event seeds.
         */
        std::vector<long> getSeeds() const {
            return seeds_;
        }        

        /**
         * Is this a real data event?
         * @return True if this is a real data event.
         */
        bool isRealData() const {
            return isRealData_;
        }

        /**
         * Set the event number.
         * @param eventNumber The event number.
         */
        void setEventNumber(int eventNumber) {
            this->eventNumber_ = eventNumber;
        }

        /**
         * Set the run number.
         * @param run The run number.
         */
        void setRun(int run) {
            this->run_ = run;
        }

        /**
         * Set the timestamp.
         * @param timestamp The timestamp.
         */
        void setTimestamp(const TTimeStamp& timestamp) {
            this->timestamp_ = timestamp;
        }

        /**
         * Set the event weight.
         * @param weight The event weight.
         */
        void setWeight(double weight) {
            this->weight_ = weight;
        }

        /**
         * Set the event seeds.
         * @param seeds The seeds.
         */
        void setSeeds(std::vector<long> seeds) {
            this->seeds_ = seeds;
        }        

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
         * The event timestamp
         */
        TTimeStamp timestamp_{0, 0};

        /**
         * The event weight.
         */
        double weight_{1.0};

        /**
         * The event seeds
         */
        std::vector<long> seeds_{ {} };        

        /**
         * Is this event real data?
         */
        bool isRealData_{false};

        /**
         * ROOT class definition.
         */
        ClassDef(EventHeader, 1);
};

}

#endif /* EVENT_EVENTHEADER_H_ */
