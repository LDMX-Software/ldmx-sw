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
#include <map>

class EventHeader {

public:

    EventHeader() :
        _eventNumber(0),
        _run(0),
        _timestamp(0) {
    }

    virtual ~EventHeader() {
    }

    int eventNumber() {
        return _eventNumber;
    }

    int run() {
        return _run;
    }

    int timestamp() {
        return _timestamp;
    }

    void setEventNumber(int eventNumber) {
        _eventNumber = eventNumber;
    }

    void setRun(int run) {
        _run = run;
    }

    void setTimestamp(int timestamp) {
        _timestamp = timestamp;
    }

private:
    long _eventNumber;
    long _run;
    long _timestamp;

    ClassDef(EventHeader, 1);
};

class Event : public TObject {

public:

    Event();

    virtual ~Event();

    EventHeader* eventHeader();

    void setEventHeader(EventHeader*);

    TClonesArray* collection(const std::string& collectionName);

    int collectionSize(const std::string& collectionName);

    TObject* addObject(const std::string& collectionName);

private:

    int nextCollectionIndex(const std::string& collectionName);

    EventHeader* _eventHeader;

    TClonesArray* _simParticles;
    TClonesArray* _taggerSimHits;
    TClonesArray* _recoilSimHits;
    TClonesArray* _ecalSimHits;
    TClonesArray* _hcalSimHits;

    int _nSimParticles;
    int _nTaggerSimHits;
    int _nRecoilSimHits;
    int _nEcalSimHits;
    int _nHcalSimHits;

    static int DEFAULT_COLLECTION_SIZE;

    ClassDef(Event, 1)
};

#endif
