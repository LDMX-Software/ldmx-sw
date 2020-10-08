#include "SimCore/Persist/SimParticleBuilder.h"

#include <string>

// LDMX
#include "Framework/Event.h"
#include "Framework/Exception/Exception.h"
#include "SimCore/G4CalorimeterHit.h"
#include "SimCore/G4TrackerHit.h"
#include "SimCore/UserTrackingAction.h"

// Geant4
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4SystemOfUnits.hh"
#include "G4VTrajectoryPoint.hh"

using namespace ldmx;

namespace simcore {
namespace persist {

SimParticleBuilder::SimParticleBuilder() : currentEvent_(nullptr) {
  trackMap_ = UserTrackingAction::getUserTrackingAction()->getTrackMap();
}

SimParticleBuilder::~SimParticleBuilder() {}

void SimParticleBuilder::buildSimParticles(ldmx::Event *outputEvent) {

  // Get the trajectory container for the event.
  auto trajectories{
      (const_cast<G4Event *>(currentEvent_))->GetTrajectoryContainer()};

  // Create empty SimParticle objects and create the map of track ID to
  // particles.
  std::map<int, SimParticle> outputParticleMap;
  for (auto trajectory : *trajectories->GetVector()) {
    outputParticleMap[trajectory->GetTrackID()] = SimParticle();
  }

  // Fill information into the particles.
  for (auto trajectory : *trajectories->GetVector()) {

    Trajectory *traj = static_cast<Trajectory *>(trajectory);

    SimParticle *simParticle = &outputParticleMap[traj->GetTrackID()];

    simParticle->setGenStatus(traj->getGenStatus());
    simParticle->setPdgID(traj->GetPDGEncoding());
    simParticle->setCharge(traj->GetCharge());
    simParticle->setMass(traj->getMass());
    simParticle->setEnergy(traj->getEnergy());
    simParticle->setTime(traj->getGlobalTime());
    simParticle->setProcessType(traj->getProcessType());
    simParticle->setVertexVolume(traj->getVertexVolume());

    const G4ThreeVector &vertex = traj->getVertexPosition();
    simParticle->setVertex(vertex[0], vertex[1], vertex[2]);

    const G4ThreeVector &momentum = traj->GetInitialMomentum();
    simParticle->setMomentum(momentum[0], momentum[1], momentum[2]);

    const G4ThreeVector &endpMomentum = traj->getEndPointMomentum();
    simParticle->setEndPointMomentum(endpMomentum[0], endpMomentum[1],
                                     endpMomentum[2]);

    G4ThreeVector endpoint = traj->getEndPoint();
    simParticle->setEndPoint(endpoint[0], endpoint[1], endpoint[2]);

    if (traj->GetParentID() > 0) {
      simParticle->addParent(traj->GetParentID());
      // check if parent track is being stored
      auto parentParticle = outputParticleMap.find(traj->GetParentID());
      if (parentParticle != outputParticleMap.end()) {
        // this parent has been found in the particle map
        outputParticleMap[traj->GetParentID()].addDaughter(
            trajectory->GetTrackID());
      } // check if parent exists in output map
    }   // check if particle has a parent
  }

  // Add the collection data to the output event.
  outputEvent->add("SimParticles", outputParticleMap);
}

} // namespace persist
} // namespace simcore
