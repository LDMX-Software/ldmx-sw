#include "SimApplication/TrackSummary.h"

// Geant4
//#include "G4DecayProducts.hh"

TrackSummary::TrackSummaryMap TrackSummary::trackMap;
TrackSummary::TrackSummaryList TrackSummary::trackList;

TrackSummary::TrackSummary(const G4Track* aTrack)
    : genStatus(0),
      simStatus(0),
      saveFlag(true),
      parentInfo(NULL) {

    charge = aTrack->GetDefinition()->GetPDGCharge();
    mass = aTrack->GetDynamicParticle()->GetMass();
    pdgID = aTrack->GetDefinition()->GetPDGEncoding();
    trackID = aTrack->GetTrackID();
    parentID = aTrack->GetParentID();
    momentum = aTrack->GetMomentum();
    energy = aTrack->GetTotalEnergy();
    globalTime = aTrack->GetGlobalTime();

    trackMap[trackID] = this;
    trackList.push_back(this);
}

TrackSummary::~TrackSummary() {
}

void TrackSummary::update(const G4Track* aTrack) {

    this->vertex = aTrack->GetVertexPosition();
    this->endPoint = aTrack->GetPosition();

    /*
    if (this->parentID == 0) {
        const G4DecayProducts* preAssignedDecayProducts = aTrack->GetDynamicParticle()->GetPreAssignedDecayProducts();
        if (preAssignedDecayProducts && preAssignedDecayProducts->entries() > 0) {
            genStatus = 2;
        } else {
            genStatus = 1;
        }
    }
    */
}

G4double TrackSummary::getCharge() const {
    return charge;
}

G4float TrackSummary::getGlobalTime() const {
    return globalTime;
}

const G4ThreeVector& TrackSummary::getEndPoint() const {
    return endPoint;
}

G4double TrackSummary::getEnergy() const {
    return energy;
}

G4double TrackSummary::getMass() const {
    return mass;
}

G4int TrackSummary::getGenStatus() const {
    return genStatus;
}

G4int TrackSummary::getSimStatus() const {
    return simStatus;
}

const G4ThreeVector& TrackSummary::getMomentum() const {
    return momentum;
}

G4int TrackSummary::getPDG() const {
    return pdgID;
}

const G4ThreeVector& TrackSummary::getVertex() const {
    return vertex;
}

G4int TrackSummary::getTrackID() const {
    return trackID;
}

G4int TrackSummary::getParentID() const {
    return parentID;
}

G4bool TrackSummary::getSaveFlag() const {
    return saveFlag;
}

void TrackSummary::setSaveFlag(bool aSaveFlag) {
    saveFlag = aSaveFlag;
}

TrackSummary* TrackSummary::findParent() {
    if (this->parentInfo == NULL) {
        this->parentInfo = trackMap[this->parentID];
    }
    return this->parentInfo;
}

TrackSummary::TrackSummaryMap* TrackSummary::getTrackSummaryMap() {
    return &trackMap;
}

TrackSummary::TrackSummaryList* TrackSummary::getTrackSummaryList() {
    return &trackList;
}

void TrackSummary::clearRegistry() {
    for (TrackSummaryList::iterator it = trackList.begin(); it != trackList.end(); it++) {
        delete *it;
    }
    trackMap.clear();
    trackList.clear();
}
