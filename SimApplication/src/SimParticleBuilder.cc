#include "SimApplication/SimParticleBuilder.h"

// LDMX
#include "Event/RootEventWriter.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/G4TrackerHit.h"
#include "SimApplication/TrajectoryContainer.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4VTrajectoryPoint.hh"

SimParticleBuilder::SimParticleBuilder() {
    trackMap = TrackMap::getInstance();
}

SimParticleBuilder::~SimParticleBuilder() {
}

void SimParticleBuilder::buildSimParticles() {

    particleMap.clear();

    RootEventWriter* writer = RootEventWriter::getInstance();
    Event* event = writer->getEvent();

    G4Event* anEvent = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
    TrajectoryContainer* trajectories = (TrajectoryContainer*)anEvent->GetTrajectoryContainer();
    for (int iTraj = 0; iTraj < trajectories->entries(); iTraj++) {
        SimParticle* simParticle = (SimParticle*) event->addObject(Event::SIM_PARTICLES);
        Trajectory* traj = (Trajectory*)(*trajectories)[iTraj];
        buildSimParticle(simParticle, traj);
        particleMap[traj->GetTrackID()] = simParticle;
    }
}

void SimParticleBuilder::buildSimParticle(SimParticle* simParticle, Trajectory* traj) {

    simParticle->setGenStatus(traj->getGenStatus());
    simParticle->setSimStatus(traj->getSimStatus());
    simParticle->setPdg(traj->GetPDGEncoding());
    simParticle->setCharge(traj->GetCharge());
    simParticle->setMass(traj->getMass() / GeV);
    simParticle->setEnergy(traj->getEnergy());

    G4ThreeVector lastTrajPoint = traj->GetPoint(traj->GetPointEntries() - 1)->GetPosition();
    simParticle->setEndPoint(lastTrajPoint[0], lastTrajPoint[1], lastTrajPoint[2]);

    const G4ThreeVector& momentum = traj->GetInitialMomentum();
    simParticle->setMomentum(momentum[0], momentum[1], momentum[2]);

    const G4ThreeVector& vertex = traj->getVertexPosition();
    simParticle->setVertex(vertex[0], vertex[1], vertex[2]);

    simParticle->setTime(traj->getGlobalTime());

    if (traj->GetParentID() > 0) {
        SimParticle* parent = findSimParticle(traj->GetParentID());
        if (parent != NULL) {
            simParticle->addParent(parent);
            parent->addDaughter(simParticle);
        } else {
            std::cerr << "WARNING: SimParticle with parent ID " << traj->GetParentID()
                << " not found for track ID " << traj->GetTrackID() << std::endl;
        }
    }
}

SimParticle* SimParticleBuilder::findSimParticle(G4int trackID) {
    G4VTrajectory* traj = trackMap->findTrajectory(trackID);
    if (traj != NULL) {
        return particleMap[traj->GetTrackID()];
    } else {
        return NULL;
    }
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
                if (trackID > 0) {
                    SimParticle* simParticle = findSimParticle(trackID);
                    if (simParticle != NULL) {
                        hit->getSimTrackerHit()->setSimParticle(simParticle);
                    } else {
                        std::cerr << "WARNING: Failed to find SimParticle for SimTrackerHit with track ID " << trackID << std::endl;
                    }
                }
            }
        }
    }
}

void SimParticleBuilder::assignCalorimeterHitSimParticles() {
    G4HCofThisEvent* hce =
            G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetHCofThisEvent();
    int nColl = hce->GetNumberOfCollections();
    for (int iColl = 0; iColl < nColl; iColl++) {
        G4VHitsCollection* hitsColl = hce->GetHC(iColl);
        G4CalorimeterHitsCollection* calHits = dynamic_cast<G4CalorimeterHitsCollection*>(hitsColl);
        if (calHits != NULL) {
            int nHits = calHits->GetSize();
            for (int iHit = 0; iHit < nHits; iHit++) {
                G4CalorimeterHit* hit = (G4CalorimeterHit*) calHits->GetHit(iHit);
                int trackID = hit->getTrackID();
                if (trackID > 0) {
                    SimParticle* simParticle = findSimParticle(trackID);
                    if (simParticle != NULL) {
                        hit->getSimCalorimeterHit()->setSimParticle(simParticle);
                    } else {
                        std::cerr << "WARNING: Failed to find SimParticle for SimCalorimeterHit with track ID " << trackID << std::endl;
                    }
                }
            }
        }
    }
}
