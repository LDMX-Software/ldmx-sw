#include "Event/Event.h"

#include "Event/EventConstants.h"

ClassImp(event::Event)

namespace event {

Event::Event() :
        TObject(),
        eventNumber(-1),
        run(-1),
        timestamp(-1),
        weight(1.0) {
}

Event::~Event() {

    Clear();

    for (CollectionMap::iterator iColl = collMap.begin();
            iColl != collMap.end(); iColl++) {
        delete (*iColl).second;
    }

    collMap.clear();
}

void Event::Clear(Option_t*) {

    TObject::Clear();
    
    eventNumber = -1;
    run = -1;
    timestamp = -1;
    weight = 1.0;

    for (CollectionMap::iterator iColl = collMap.begin();
                iColl != collMap.end(); iColl++) {
        (*iColl).second->Clear("C");
    }
}


int Event::getEventNumber() {
    return eventNumber;
}

int Event::getRun() {
    return run;
}

int Event::getTimestamp() {
    return timestamp;
}

double Event::getWeight() {
    return weight;
}

void Event::setEventNumber(int anEventNumber) {
    eventNumber = anEventNumber;
}

void Event::setRun(int aRun) {
    run = aRun;
}

void Event::setTimestamp(int aTimestamp) {
    timestamp = aTimestamp;
}

void Event::setWeight(double aWeight) {
    weight = aWeight;
}

TClonesArray* Event::getCollection(const std::string& collectionName) {
    return collMap[collectionName];
}

}
