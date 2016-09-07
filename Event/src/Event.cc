#include "Event/Event.h"

ClassImp(Event)

int Event::DEFAULT_COLLECTION_SIZE = 1000;

Event::Event() :
        TObject(),
        _eventHeader(0),
        _simParticles(new TClonesArray("SimParticle", Event::DEFAULT_COLLECTION_SIZE)),
        _taggerSimHits(new TClonesArray("SimTrackerHit", Event::DEFAULT_COLLECTION_SIZE)),
        _recoilSimHits(new TClonesArray("SimTrackerHit", Event::DEFAULT_COLLECTION_SIZE)),
        _ecalSimHits(new TClonesArray("SimCalorimeterHit", Event::DEFAULT_COLLECTION_SIZE)),
        _hcalSimHits(new TClonesArray("SimCalorimeterHit", Event::DEFAULT_COLLECTION_SIZE)),
        _nSimParticles(0),
        _nTaggerSimHits(0),
        _nRecoilSimHits(0),
        _nEcalSimHits(0),
        _nHcalSimHits(0) {
}

Event::~Event() {
    Clear();
/*
    delete _simParticles;
    delete _taggerSimHits;
    delete _recoilSimHits;
    delete _ecalSimHits;
    delete _hcalSimHits;
*/
}

void Event::setEventHeader(EventHeader* eventHeader) {
    _eventHeader = eventHeader;
}

EventHeader* Event::eventHeader() {
    return _eventHeader;
}

int Event::collectionSize(const std::string& collectionName) {
    if (collectionName.compare("SimParticles") == 0) {
        return _nSimParticles;
    } else if (collectionName.compare("TaggerSimHits") == 0) {
        return _nTaggerSimHits;
    } else if (collectionName.compare("RecoilSimHits")) {
        return _nRecoilSimHits;
    } else if (collectionName.compare("EcalSimHits")) {
        return _nEcalSimHits;
    } else if (collectionName.compare("HcalSimHits")) {
        return _nHcalSimHits;
    } else {
        return -1;
    }
}

int Event::nextCollectionIndex(const std::string& collectionName) {
    if (collectionName.compare("SimParticles") == 0) {
        return _nSimParticles++;
    } else if (collectionName.compare("TaggerSimHits") == 0) {
        return _nTaggerSimHits++;
    } else if (collectionName.compare("RecoilSimHits")) {
        return _nRecoilSimHits++;
    } else if (collectionName.compare("EcalSimHits")) {
        return _nEcalSimHits++;
    } else if (collectionName.compare("HcalSimHits")) {
        return _nHcalSimHits;
    } else {
        return -1;
    }
}

TClonesArray* Event::collection(const std::string& collectionName) {
    if (collectionName.compare("SimParticles") == 0) {
        return _simParticles;
    } else if (collectionName.compare("TaggerSimHits") == 0) {
        return _taggerSimHits;
    } else if (collectionName.compare("RecoilSimHits")) {
        return _recoilSimHits;
    } else if (collectionName.compare("EcalSimHits")) {
        return _ecalSimHits;
    } else if (collectionName.compare("HcalSimHits")) {
        return _hcalSimHits;
    } else {
        return 0;
    }
}

TObject* Event::addObject(const std::string& collectionName) {
    TClonesArray* coll = collection(collectionName);
    if (coll != 0) {
        return coll->ConstructedAt(nextCollectionIndex(collectionName));
    } else {
        return 0;
    }
}
