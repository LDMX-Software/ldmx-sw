#include "Event/Event.h"

ClassImp(event::Event)

namespace event {

int Event::DEFAULT_COLLECTION_SIZE = 100;

std::string Event::SIM_PARTICLES = std::string("SimParticles");
std::string Event::RECOIL_SIM_HITS = std::string("RecoilSimHits");
std::string Event::TAGGER_SIM_HITS = std::string("TaggerSimHits");
std::string Event::ECAL_SIM_HITS = std::string("EcalSimHits");
std::string Event::HCAL_SIM_HITS = std::string("HcalSimHits");

Event::Event() :
        TObject(),
        eventNumber(-1),
        run(-1),
        timestamp(-1),
        weight(1.0),
        simParticles(new TClonesArray("SimParticle", Event::DEFAULT_COLLECTION_SIZE)),
        taggerSimHits(new TClonesArray("SimTrackerHit", Event::DEFAULT_COLLECTION_SIZE)),
        recoilSimHits(new TClonesArray("SimTrackerHit", Event::DEFAULT_COLLECTION_SIZE)),
        ecalSimHits(new TClonesArray("SimCalorimeterHit", Event::DEFAULT_COLLECTION_SIZE)),
        hcalSimHits(new TClonesArray("SimCalorimeterHit", Event::DEFAULT_COLLECTION_SIZE)),
        nSimParticles(0),
        nTaggerSimHits(0),
        nRecoilSimHits(0),
        nEcalSimHits(0),
        nHcalSimHits(0) {
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

    nSimParticles = 0;
    nTaggerSimHits = 0;
    nRecoilSimHits = 0;
    nEcalSimHits = 0;
    nHcalSimHits = 0;
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

int Event::getCollectionSize(const std::string& collectionName) {
    if (collectionName.compare(SIM_PARTICLES) == 0) {
        return nSimParticles;
    } else if (collectionName.compare(TAGGER_SIM_HITS) == 0) {
        return nTaggerSimHits;
    } else if (collectionName.compare(RECOIL_SIM_HITS) == 0) {
        return nRecoilSimHits;
    } else if (collectionName.compare(ECAL_SIM_HITS) == 0) {
        return nEcalSimHits;
    } else if (collectionName.compare(HCAL_SIM_HITS) == 0) {
        return nHcalSimHits;
    } else {
        throw std::runtime_error("Unknown collection name: " + collectionName);
    }
}

int Event::nextCollectionIndex(const std::string& collectionName) {
    if (collectionName.compare(SIM_PARTICLES) == 0) {
        return nSimParticles++;
    } else if (collectionName.compare(TAGGER_SIM_HITS) == 0) {
        return nTaggerSimHits++;
    } else if (collectionName.compare(RECOIL_SIM_HITS) == 0) {
        return nRecoilSimHits++;
    } else if (collectionName.compare(ECAL_SIM_HITS) == 0) {
        return nEcalSimHits++;
    } else if (collectionName.compare(HCAL_SIM_HITS) == 0) {
        return nHcalSimHits++;
    } else {
        throw std::runtime_error("Unknown collection name: " + collectionName);
    }
}

TClonesArray* Event::getCollection(const std::string& collectionName) {
    if (collectionName.compare(SIM_PARTICLES) == 0) {
        return simParticles;
    } else if (collectionName.compare(TAGGER_SIM_HITS) == 0) {
        return taggerSimHits;
    } else if (collectionName.compare(RECOIL_SIM_HITS) == 0) {
        return recoilSimHits;
    } else if (collectionName.compare(ECAL_SIM_HITS) == 0) {
        return ecalSimHits;
    } else if (collectionName.compare(HCAL_SIM_HITS) == 0) {
        return hcalSimHits;
    } else {
        throw std::runtime_error("Unknown collection name: " + collectionName);
    }
}

TObject* Event::addObject(const std::string& collectionName) {
    TClonesArray* coll = getCollection(collectionName);
    return coll->ConstructedAt(nextCollectionIndex(collectionName));
}

}
