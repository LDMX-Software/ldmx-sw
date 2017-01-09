#include "SimApplication/SimParticleBuilder.h"

// LDMX
#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "Event/RootEventWriter.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/G4TrackerHit.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4VTrajectoryPoint.hh"

using event::Event;
using event::EventConstants;
using event::RootEventWriter;

namespace sim {

SimParticleBuilder::SimParticleBuilder() : currentEvent_(nullptr) {
    trackMap_ = TrackMap::getInstance();
}

SimParticleBuilder::~SimParticleBuilder() {
}

void SimParticleBuilder::buildSimParticles(Event* outputEvent) {

    TrajectoryContainer* trajectories;
    if (currentEvent_->GetTrajectoryContainer() != nullptr) {
        trajectories = (TrajectoryContainer*)(const_cast<G4Event*>(currentEvent_))->GetTrajectoryContainer();
    } else {
        G4Exception("SimParticleBuilder::buildSimParticles",
                "",
                FatalException,
                "TrajectoryContainer for the event is null.");
    }

    TClonesArray* coll = outputEvent->getCollection(EventConstants::SIM_PARTICLES);
    buildParticleMap(trajectories, coll); 
    for (auto trajectory : *trajectories->GetVector()) { 
        buildSimParticle(static_cast<Trajectory*>(trajectory));
    }
}

void SimParticleBuilder::buildSimParticle(Trajectory* traj) {

    SimParticle* simParticle = particleMap_[traj->GetTrackID()];

    if (!simParticle) {
        std::cerr << "SimParticle not found for Trajectory with track ID "
                << traj->GetTrackID() << std::endl;
        G4Exception("SimParticleBuilder::buildSimParticle",
                "",
                FatalException,
                "SimParticle not found for Trajectory.");
    }

    simParticle->setGenStatus(traj->getGenStatus());
    simParticle->setPdgID(traj->GetPDGEncoding());
    simParticle->setCharge(traj->GetCharge());
    simParticle->setMass(traj->getMass());
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
            std::cerr << "[ SimParticleBuilder ]: WARNING: SimParticle with parent ID " 
                      << traj->GetParentID() << " not found for track ID " 
                      << traj->GetTrackID() << std::endl;
        }
    }
}

void SimParticleBuilder::buildParticleMap(TrajectoryContainer* trajectories, TClonesArray* simParticleColl) { 
    particleMap_.clear();
    for (auto trajectory : *trajectories->GetVector()) { 
        particleMap_[trajectory->GetTrackID()] 
            = (SimParticle*) simParticleColl->ConstructedAt(simParticleColl->GetEntries());
    }
}

SimParticle* SimParticleBuilder::findSimParticle(G4int trackID) {
    G4VTrajectory* traj = trackMap_->findTrajectory(currentEvent_, trackID);
    if (traj != NULL) {
        return particleMap_[traj->GetTrackID()];
    } else {
        return NULL;
    }
}

}
