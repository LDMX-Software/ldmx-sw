#include "Event/Event.h"

#include "Event/EventConstants.h"

ClassImp(event::Event)

namespace event {

Event::Event() :
        TObject(),
        eventNumber(-1),
        run(-1),
        timestamp(-1),
        weight(1.0),
        simParticles(new TClonesArray("event::SimParticle", event::DEFAULT_COLLECTION_SIZE)),
        taggerSimHits(new TClonesArray("event::SimTrackerHit", event::DEFAULT_COLLECTION_SIZE)),
        recoilSimHits(new TClonesArray("event::SimTrackerHit", event::DEFAULT_COLLECTION_SIZE)),
        ecalSimHits(new TClonesArray("event::SimCalorimeterHit", event::DEFAULT_COLLECTION_SIZE)),
        hcalSimHits(new TClonesArray("event::SimCalorimeterHit", event::DEFAULT_COLLECTION_SIZE)) {

    // Map names to collections.
    collectionMap[RECOIL_SIM_HITS] = recoilSimHits;
    collectionMap[TAGGER_SIM_HITS] = taggerSimHits;
    collectionMap[SIM_PARTICLES] = simParticles;
    collectionMap[ECAL_SIM_HITS] = ecalSimHits;
    collectionMap[HCAL_SIM_HITS] = hcalSimHits;
}

Event::~Event() {

    Clear();

    delete simParticles;
    delete taggerSimHits;
    delete recoilSimHits;
    delete ecalSimHits;
    delete hcalSimHits;
}

void Event::Clear(Option_t*) {

    TObject::Clear();
    
    eventNumber = -1;
    run = -1;
    timestamp = -1;
    weight = 1.0;

    simParticles->Clear("C");
    taggerSimHits->Clear("C");
    recoilSimHits->Clear("C");
    ecalSimHits->Clear("C");
    hcalSimHits->Clear("C");
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
    return collectionMap[collectionName];
}

}
