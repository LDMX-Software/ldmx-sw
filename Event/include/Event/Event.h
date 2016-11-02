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

namespace event {

class Event: public TObject {

    public:

        typedef std::map<std::string, TClonesArray*> CollectionMap;

        Event();

        virtual ~Event();

        void Clear(Option_t* = "");

        int getEventNumber() { return eventNumber; }

        int getRun() { return run; }

        int getTimestamp() { return timestamp; }
        
        double getWeight() { return weight; }

        void setEventNumber(int eventNumber) { this->eventNumber = eventNumber; }

        void setRun(int run) { this->run = run; }

        void setTimestamp(int timestamp) { this->timestamp = timestamp; }
        
        void setWeight(double weight) { this->weight = weight; }

        TClonesArray* getCollection(const std::string& collectionName) {
            return collMap[collectionName];
        }

        const CollectionMap& getCollectionMap() {
            return collMap;
        }

        /**
         * Concrete sub-classes must implement this method to return a string
         * with the class name of the event type e.g. "event::SimEvent".
         */
        virtual const char* getEventType() = 0;

    private:

        int eventNumber{-1};
        int run{-1};
        int timestamp{-1};
        double weight{1.0};

    protected:

        CollectionMap collMap; //!

        ClassDef(Event, 1);
};

}

#endif
