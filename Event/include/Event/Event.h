/**
 * @file Event.h
 * @brief Class defining an abstract interface for accessing event information and data collections
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_EVENT_H_
#define EVENT_EVENT_H_

// ROOT
#include "TObject.h"
#include "TClonesArray.h"

// LDMX
#include "Event/EventConstants.h"
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

// STL
#include <string>
#include <map>

/**
 * @namespace event
 * @brief %Event model classes
 */
namespace event {

/**
 * @class Event
 * @brief Abstract interface for accessing event data
 *
 * @note
 * This is the base class which is extended by concrete event types.
 * It provides access to event number, run number, timestamp and
 * event weight.  Collections contained in <i>TClonesArray</i> objects
 * are accessible by name.  Concrete sub-types should provide the actual
 * collections.
 *
 * @see SimEvent
 */
class Event: public TObject {

    public:

        typedef std::map<std::string, TClonesArray*> CollectionMap;

        /**
         * Class constructor.
         */
        Event();

        /**
         * Class destructor.
         */
        virtual ~Event();

        /**
         * Clear information from this event including the data collections.
         */
        void Clear(Option_t* = "");

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

        /**
         * Get a reference to the <i>TClonesArray</i> containing collection data.
         * @param collectionName The name of the collection.
         */
        TClonesArray* getCollection(const std::string& collectionName) {
            return collMap_[collectionName];
        }

        /**
         * Get a map of names to collections.
         * @return A map of names to collections.
         */
        const CollectionMap& getCollectionMap() const {
            return collMap_;
        }

        /**
         * Add an object to a collection.
         * The object will have the specific type which was set when the
         * collection was defined by a <i>TClonesArray</i>.
         * @return A pointer to the new object.
         */
        TObject* addObject(const std::string& collectionName) {
            auto coll = getCollection(collectionName);
            return coll->ConstructedAt(coll->GetEntriesFast());
        }

        /**
         * Concrete sub-classes must implement this method to return a string
         * with the class name of the event type e.g. "event::SimEvent".
         */
        virtual const std::string& getEventType() = 0;

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

    protected:

        /**
         * Map of names to collections.
         */
        CollectionMap collMap_; //!

    /**
     * ROOT class definition.
     */
    ClassDef(Event, 1);
};

}

#endif
