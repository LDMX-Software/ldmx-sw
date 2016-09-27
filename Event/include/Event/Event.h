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

        void setEventNumber(int anEventNumber) {
            eventNumber = anEventNumber;
        }

        void setRun(int aRun) {
            run = aRun;
        }

        void setTimestamp(int aTimestamp) {
            timestamp = aTimestamp;
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

    public:

        static std::string SIM_PARTICLES;
        static std::string RECOIL_SIM_HITS;
        static std::string TAGGER_SIM_HITS;
        static std::string ECAL_SIM_HITS;
        static std::string HCAL_SIM_HITS;

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
