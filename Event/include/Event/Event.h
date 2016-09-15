#ifndef EVENT_EVENT_H_
#define EVENT_EVENT_H_ 1

// ROOT
#include "TObject.h"
#include "TClonesArray.h"

// LDMX
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

// STL
#include <string>

class EventHeader {

    public:

        EventHeader() :
                eventNumber(0), run(0), timestamp(0) {
        }

        virtual ~EventHeader() {
        }

        int getEventNumber() {
            return eventNumber;
        }

        int getRun() {
            return run;
        }

        int getTimestamp() {
            return timestamp;
        }

        void setEventNumber(int eventNumber) {
            this->eventNumber = eventNumber;
        }

        void setRun(int run) {
            this->run = run;
        }

        void setTimestamp(int timestamp) {
            this->timestamp = timestamp;
        }

    private:
        int eventNumber;
        int run;
        int timestamp;

    ClassDef(EventHeader, 1);
};

class Event: public TObject {

    public:

        Event();

        virtual ~Event();

        void Clear(Option_t* = "");

        EventHeader* getHeader();

        TClonesArray* getCollection(const std::string& collectionName);

        int getCollectionSize(const std::string& collectionName);

        TObject* addObject(const std::string& collectionName);

    private:

        int nextCollectionIndex(const std::string& collectionName);

        EventHeader header;

        TClonesArray* simParticles;
        TClonesArray* taggerSimHits;
        TClonesArray* recoilSimHits;
        TClonesArray* ecalSimHits;
        TClonesArray* hcalSimHits;

        int nSimParticles;
        int nTaggerSimHits;
        int nRecoilSimHits;
        int nEcalSimHits;
        int nHcalSimHits;

        static int DEFAULT_COLLECTION_SIZE;

        ClassDef(Event, 1);
};

#endif
