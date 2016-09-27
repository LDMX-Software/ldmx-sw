#include "SimApplication/SimParticleBuilder.h"

// LDMX
#include "SimApplication/G4TrackerHit.h"
#include "SimApplication/UserTrackInformation.h"
#include "Event/RootEventWriter.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"

SimParticleBuilder::SimParticleBuilder() {
}

SimParticleBuilder::~SimParticleBuilder() {
}

void SimParticleBuilder::buildSimParticles() {

    particleMap.clear();

    TrackSummary::TrackSummaryList* trackList =
            TrackSummary::getTrackSummaryList();

    RootEventWriter* writer = RootEventWriter::getInstance();
    Event* event = writer->getEvent();

    for (TrackSummary::TrackSummaryList::iterator it = trackList->begin();
            it != trackList->end(); it++) {
        SimParticle* p = (SimParticle*) event->addObject(Event::SIM_PARTICLES);
        TrackSummary* trackSummary = *it;
        buildSimParticle(p, trackSummary);
        particleMap[trackSummary->getTrackID()] = p;
    }
}

void SimParticleBuilder::buildSimParticle(SimParticle* simParticle, TrackSummary* trackSummary) {

    simParticle->setGenStatus(trackSummary->getGenStatus());
    simParticle->setSimStatus(trackSummary->getSimStatus());
    simParticle->setPdg(trackSummary->getPDG());
    simParticle->setCharge(trackSummary->getCharge());
    simParticle->setMass(trackSummary->getMass() / GeV);
    simParticle->setEnergy(trackSummary->getEnergy());

    const G4ThreeVector& endPoint = trackSummary->getEndPoint();
    simParticle->setEndPoint(endPoint[0], endPoint[1], endPoint[2]);

    const G4ThreeVector& momentum = trackSummary->getMomentum();
    simParticle->setMomentum(momentum[0], momentum[1], momentum[2]);

    const G4ThreeVector& vertex = trackSummary->getVertex();
    simParticle->setVertex(vertex[0], vertex[1], vertex[2]);

    simParticle->setTime(trackSummary->getGlobalTime());

    SimParticle* parent = particleMap[trackSummary->getParentID()];
    if (parent != NULL) {
        simParticle->addParent(parent);
        parent->addDaughter(simParticle);
    }
}

SimParticle* SimParticleBuilder::findSimParticle(G4int trackID) {
    return particleMap[trackID];
}

void SimParticleBuilder::assignTrackerHitSimParticles() {
    G4HCofThisEvent* hce =
            G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetHCofThisEvent();
    int nColl = hce->GetNumberOfCollections();
    for (int iColl = 0; iColl < nColl; iColl++) {
        G4VHitsCollection* hitsColl = hce->GetHC(iColl);
        G4TrackerHitsCollection* trackerHits = dynamic_cast<G4TrackerHitsCollection*>(hitsColl);
        if (trackerHits != NULL) {
            int nHits = trackerHits->GetSize();
            for (int iHit = 0; iHit < nHits; iHit++) {
                G4TrackerHit* hit = (G4TrackerHit*) trackerHits->GetHit(iHit);
                int trackID = hit->getTrackID();
                if (trackID >= 0) {
                    SimParticle* simParticle = findSimParticle(trackID);
                    if (simParticle != NULL) {
                        //std::cout << "Assigning SimParticle with track ID " << trackID << " to SimTrackerHit" << std::endl;
                        hit->getSimTrackerHit()->setSimParticle(simParticle);
                    }
                }
            }
        }
    }
}
