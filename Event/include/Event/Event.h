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

        int getEventNumber() { return eventNumber_; }

        int getRun() { return run_; }

        int getTimestamp() { return timestamp_; }
        
        double getWeight() { return weight_; }

        void setEventNumber(int eventNumber) { this->eventNumber_ = eventNumber; }

        void setRun(int run) { this->run_ = run; }

        void setTimestamp(int timestamp) { this->timestamp_ = timestamp; }
        
        void setWeight(double weight) { this->weight_ = weight; }

        TClonesArray* getCollection(const std::string& collectionName) {
            return collMap_[collectionName];
        }

        const CollectionMap& getCollectionMap() {
            return collMap_;
        }

        TObject* addObject(const std::string& collectionName) {
            auto coll = getCollection(collectionName);
            return coll->ConstructedAt(coll->GetEntriesFast());
        }

        /**
         * Concrete sub-classes must implement this method to return a string
         * with the class name of the event type e.g. "event::SimEvent".
         */
        virtual const char* getEventType() = 0;

    private:

        int eventNumber_{-1};
        int run_{-1};
        int timestamp_{-1};
        double weight_{1.0};

    protected:

        CollectionMap collMap_; //!

        ClassDef(Event, 1);
};

}

#endif
